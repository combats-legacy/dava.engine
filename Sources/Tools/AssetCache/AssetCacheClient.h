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

#ifndef __ASSET_CACHE_CLIENT_H__
#define __ASSET_CACHE_CLIENT_H__

#include "AssetCache/AssetCache.h"
#include <atomic>

namespace DAVA
{
class AssetCacheClient final : public AssetCache::ClientNetProxyListener
{
    struct ResultOfRequest
    {
        int32 requestID = AssetCache::PACKET_UNKNOWN;
        bool recieved = false;
        bool succeed = false;
        bool processingRequest = false;
    };

public:
    struct ConnectionParams
    {
        String ip = "127.0.0.1";
        uint16 port = AssetCache::ASSET_SERVER_PORT;
        uint64 timeoutms = 60u * 1000u;
    };

    AssetCacheClient(bool emulateNetworkLoop);
    ~AssetCacheClient() override;

    AssetCache::ErrorCodes ConnectBlocked(const ConnectionParams& connectionParams);
    void Disconnect();

    AssetCache::ErrorCodes AddToCacheBlocked(const AssetCache::CacheItemKey& key, const AssetCache::CachedItemValue& value);
    AssetCache::ErrorCodes RequestFromCacheBlocked(const AssetCache::CacheItemKey& key, const FilePath& outFolder);

    bool IsConnected() const;

private:
    void ProcessNetwork();

    AssetCache::ErrorCodes WaitRequest();

    //ClientNetProxyListener
    void OnAddedToCache(const AssetCache::CacheItemKey& key, bool added) override;
    void OnReceivedFromCache(const AssetCache::CacheItemKey& key, const AssetCache::CachedItemValue& value) override;
    void OnAssetClientStateChanged() override;

private:
    AssetCache::ClientNetProxy client;

    uint64 timeoutms = 60u * 1000u;

    Mutex requestLocker;
    ResultOfRequest requestResult;

    UnorderedMap<AssetCache::CacheItemKey, FilePath> requests;
    std::atomic<bool> isActive;
    std::atomic<bool> isJobStarted;

    bool emulateNetworkLoop = false;
};

} //END of DAVA

#endif //__ASSET_CACHE_CLIENT_H__
