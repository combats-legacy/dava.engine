/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "AssetCacheClient.h"

#include "FileSystem/FileSystem.h"
#include "Platform/SystemTimer.h"
#include "Concurrency/LockGuard.h"
#include "Concurrency/Thread.h"
#include "Job/JobManager.h"

namespace DAVA
{
AssetCacheClient::AssetCacheClient(bool emulateNetworkLoop_)
    : isActive(false)
    , isJobStarted(false)
    , emulateNetworkLoop(emulateNetworkLoop_)
{
    DVASSERT(JobManager::Instance() != nullptr);

    client.AddListener(this);
}

AssetCacheClient::~AssetCacheClient()
{
    client.RemoveListener(this);

    DVASSERT(isActive == false);
    DVASSERT(isJobStarted == false);
}

AssetCache::Error AssetCacheClient::ConnectSynchronously(const ConnectionParams& connectionParams)
{
    timeoutms = connectionParams.timeoutms;

    isActive = true;
    if (emulateNetworkLoop)
    {
        JobManager::Instance()->CreateWorkerJob(MakeFunction(this, &AssetCacheClient::ProcessNetwork));
    }

    bool connectCalled = client.Connect(connectionParams.ip, connectionParams.port);
    if (!connectCalled)
    {
        isActive = false;
        return AssetCache::Error::ADDRESS_RESOLVER_FAILED;
    }

    uint64 startTime = SystemTimer::Instance()->AbsoluteMS();
    while (client.ChannelIsOpened() == false)
    {
        uint64 deltaTime = SystemTimer::Instance()->AbsoluteMS() - startTime;
        if (((timeoutms > 0) && (deltaTime > timeoutms)) && (client.ChannelIsOpened() == false))
        {
            Logger::Error("[AssetCacheClient::%s] connection to %s:%hu refused by timeout (%lld ms)", __FUNCTION__, connectionParams.ip.c_str(), connectionParams.port, connectionParams.timeoutms);
            isActive = false;
            return AssetCache::Error::OPERATION_TIMEOUT;
        }
    }

    return AssetCache::Error::NO_ERRORS;
}

void AssetCacheClient::Disconnect()
{
    isActive = false;
    client.Disconnect();

    while (isJobStarted)
    {
        //wait for finishing of networking
    }
}

AssetCache::Error AssetCacheClient::AddToCacheSynchronously(const AssetCache::CacheItemKey& key, const AssetCache::CachedItemValue& value)
{
    {
        LockGuard<Mutex> guard(requestLocker);
        request = Request(key, FilePath(), AssetCache::PACKET_ADD_REQUEST);
    }

    AssetCache::Error resultCode = AssetCache::Error::CANNOT_SEND_REQUEST_ADD;

    bool requestSent = client.AddToCache(key, value);
    if (requestSent)
    {
        resultCode = WaitRequest();
    }

    {
        LockGuard<Mutex> guard(requestLocker);
        request.Reset();
    }

    return resultCode;
}

AssetCache::Error AssetCacheClient::RequestFromCacheSynchronously(const AssetCache::CacheItemKey& key, const FilePath& outputFolder)
{
    {
        LockGuard<Mutex> guard(requestLocker);
        request = Request(key, outputFolder, AssetCache::PACKET_GET_REQUEST);
    }

    AssetCache::Error resultCode = AssetCache::Error::CANNOT_SEND_REQUEST_GET;

    bool requestSent = client.RequestFromCache(key);
    if (requestSent)
    {
        resultCode = WaitRequest();
    }

    {
        LockGuard<Mutex> guard(requestLocker);
        request.Reset();
    }

    return resultCode;
}

AssetCache::Error AssetCacheClient::WaitRequest()
{
    uint64 startTime = SystemTimer::Instance()->AbsoluteMS();

    Request currentRequest;
    {
        LockGuard<Mutex> guard(requestLocker);
        currentRequest = request;
    }

    while (currentRequest.recieved == false)
    {
        {
            LockGuard<Mutex> guard(requestLocker);
            currentRequest = request;
        }

        auto deltaTime = SystemTimer::Instance()->AbsoluteMS() - startTime;
        if (((timeoutms > 0) && (deltaTime > timeoutms)) && (currentRequest.recieved == false) && (currentRequest.processingRequest == false))
        {
            return AssetCache::Error::OPERATION_TIMEOUT;
        }
    }

    if (currentRequest.result == AssetCache::Error::NO_ERRORS)
    {
        while (currentRequest.processingRequest)
        {
            LockGuard<Mutex> guard(requestLocker);
            currentRequest = request;
        }
    }

    return currentRequest.result;
}

void AssetCacheClient::OnAddedToCache(const AssetCache::CacheItemKey& key, bool added)
{
    LockGuard<Mutex> guard(requestLocker);
    if (request.requestID == AssetCache::PACKET_ADD_REQUEST && request.key == key)
    {
        request.result = (added) ? AssetCache::Error::NO_ERRORS : AssetCache::Error::SERVER_ERROR;
        request.recieved = true;
        request.processingRequest = false;
    }
    else
    {
        //skip this request, because it was canceled by timeout
    }
}

void AssetCacheClient::OnReceivedFromCache(const AssetCache::CacheItemKey& key, const AssetCache::CachedItemValue& value)
{
    Request currentRequest;
    {
        LockGuard<Mutex> guard(requestLocker);
        currentRequest = request;
    }

    if (currentRequest.requestID == AssetCache::PACKET_GET_REQUEST && currentRequest.key == key)
    {
        auto DumpInfo = [](const AssetCache::CacheItemKey& key, const AssetCache::CachedItemValue& value)
        {
            const AssetCache::CachedItemValue::Description& description = value.GetDescription();
            Logger::Info("[%s] %s - %s", description.creationDate.c_str(), description.machineName.c_str(), AssetCache::KeyToString(key).c_str());
            Logger::FrameworkDebug("[AssetCacheClient::OnReceivedFromCache] addingChain(%s), receivingChain(%s), comment(%s)", description.addingChain.c_str(), description.receivingChain.c_str(), description.comment.c_str());
        };

        if (value.IsEmpty())
        {
            LockGuard<Mutex> guard(requestLocker);
            request.result = AssetCache::Error::NOT_FOUND_ON_SERVER;
            request.recieved = true;
            request.processingRequest = false;
        }
        else if (value.IsValid() == false)
        {
            LockGuard<Mutex> guard(requestLocker);
            request.result = AssetCache::Error::CORRUPTED_DATA;
            request.recieved = true;
            request.processingRequest = false;

            DumpInfo(key, value);
        }
        else
        {
            { // mark request as recieved and processed
                LockGuard<Mutex> guard(requestLocker);
                request.result = AssetCache::Error::NO_ERRORS;
                request.recieved = true;
                request.processingRequest = true;
            }

            DumpInfo(key, value);

            FileSystem::Instance()->CreateDirectory(currentRequest.outputFolder, true);
            value.Export(currentRequest.outputFolder);

            { // mark request as processed
                LockGuard<Mutex> guard(requestLocker);
                request.processingRequest = false;
            }
        }
    }
    else
    {
        //skip this request, because it was canceled by timeout
    }
}

void AssetCacheClient::ProcessNetwork()
{
    isJobStarted = true;

    while (isActive)
    {
        Net::NetCore::Instance()->Poll();
    }

    isJobStarted = false;
}

void AssetCacheClient::OnAssetClientStateChanged()
{
    if (client.ChannelIsOpened() == false)
    {
        isActive = false;
    }
}

bool AssetCacheClient::IsConnected() const
{
    return client.ChannelIsOpened();
}

} //END of DAVA
