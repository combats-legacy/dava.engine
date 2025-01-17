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

#include "DynamicBufferAllocator.h"
#include "Render/RenderCallbacks.h"
#include "Functional/Function.h"

namespace DAVA
{
namespace DynamicBufferAllocator
{
namespace //for private members
{
uint32 pageSize = DEFAULT_PAGE_SIZE;

template <class HBuffer>
class BufferProxy
{
public:
    static HBuffer CreateBuffer(uint32 size);
    static uint8* MapBuffer(HBuffer handle, uint32 offset, uint32 size);
    static void UnmapBuffer(HBuffer handle);
    static void DeleteBuffer(HBuffer handle);
};

template <>
class BufferProxy<rhi::HVertexBuffer>
{
public:
    static rhi::HVertexBuffer CreateBuffer(uint32 size)
    {
        rhi::VertexBuffer::Descriptor descr = rhi::VertexBuffer::Descriptor(size);
        descr.needRestore = false;
        descr.usage = rhi::USAGE_DYNAMICDRAW;
        return rhi::CreateVertexBuffer(descr);
    }
    static uint8* MapBuffer(rhi::HVertexBuffer handle, uint32 offset, uint32 size)
    {
        return reinterpret_cast<uint8*>(rhi::MapVertexBuffer(handle, offset, size));
    }
    static void UnmapBuffer(rhi::HVertexBuffer handle)
    {
        rhi::UnmapVertexBuffer(handle);
    }
    static void DeleteBuffer(rhi::HVertexBuffer handle)
    {
        rhi::DeleteVertexBuffer(handle);
    }
};

template <>
class BufferProxy<rhi::HIndexBuffer>
{
public:
    static rhi::HIndexBuffer CreateBuffer(uint32 size)
    {
        rhi::IndexBuffer::Descriptor descr = rhi::IndexBuffer::Descriptor(size);
        descr.needRestore = false;
        descr.usage = rhi::USAGE_DYNAMICDRAW;
        return rhi::CreateIndexBuffer(descr);
    }
    static uint8* MapBuffer(rhi::HIndexBuffer handle, uint32 offset, uint32 size)
    {
        return reinterpret_cast<uint8*>(rhi::MapIndexBuffer(handle, offset, size));
    }
    static void UnmapBuffer(rhi::HIndexBuffer handle)
    {
        rhi::UnmapIndexBuffer(handle);
    }
    static void DeleteBuffer(rhi::HIndexBuffer handle)
    {
        rhi::DeleteIndexBuffer(handle);
    }
};

template <class HBuffer>
struct BufferAllocator
{
    struct BufferInfo
    {
        HBuffer buffer;
        uint32 allocatedSize;
        rhi::HSyncObject readySync;
    };

    struct BufferAllocateResult
    {
        HBuffer buffer;
        uint8* data;
        uint32 base;
        uint32 count;
    };

    BufferAllocateResult AllocateData(uint32 size, uint32 count)
    {
        DVASSERT(size);

        uint32 requiredSize = (size * count);
        if (requiredSize > pageSize)
        {
            count = pageSize / size;
            requiredSize = size * count;
        }

        uint32 base = ((currentlyUsedSize + size - 1) / size);
        uint32 offset = base * size;
        //cant fit - start new
        if ((!currentlyMappedBuffer) || ((offset + requiredSize) > currentlyMappedBuffer->allocatedSize))
        {
            if (currentlyMappedBuffer) //unmap it
            {
                buffersToUnmap.push_back(currentlyMappedBuffer);
                usedBuffers.push_back(currentlyMappedBuffer);
            }
            if (freeBuffers.size())
            {
                currentlyMappedBuffer = *freeBuffers.begin();
                freeBuffers.pop_front();
            }
            else
            {
                currentlyMappedBuffer = new BufferInfo();
                currentlyMappedBuffer->allocatedSize = pageSize;
                currentlyMappedBuffer->buffer = BufferProxy<HBuffer>::CreateBuffer(pageSize);
            }
            currentlyMappedData = BufferProxy<HBuffer>::MapBuffer(currentlyMappedBuffer->buffer, 0, currentlyMappedBuffer->allocatedSize);
            currentlyMappedBuffer->readySync = rhi::GetCurrentFrameSyncObject();
            offset = 0;
            base = 0;
        }

        BufferAllocateResult res;
        res.buffer = currentlyMappedBuffer->buffer;
        res.data = currentlyMappedData + offset;
        res.base = base;
        res.count = count;

        currentlyUsedSize = offset + requiredSize;

        return res;
    }

    void Clear()
    {
        for (auto b : buffersToUnmap)
        {
            BufferProxy<HBuffer>::UnmapBuffer(b->buffer);
        }
        for (auto b : freeBuffers)
        {
            BufferProxy<HBuffer>::DeleteBuffer(b->buffer);
            SafeDelete(b);
        }
        for (auto b : usedBuffers)
        {
            BufferProxy<HBuffer>::DeleteBuffer(b->buffer);
            SafeDelete(b);
        }

        freeBuffers.clear();
        usedBuffers.clear();
        buffersToUnmap.clear();
    }

    void BeginFrame()
    {
        //update used buffers ttl
        auto it = usedBuffers.begin();
        while (it != usedBuffers.end())
        {
            if (rhi::SyncObjectSignaled((*it)->readySync))
            {
                freeBuffers.push_back(*it);
                it = usedBuffers.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void EndFrame()
    {
        for (auto b : buffersToUnmap)
        {
            BufferProxy<HBuffer>::UnmapBuffer(b->buffer);
        }
        buffersToUnmap.clear();

        if (currentlyMappedBuffer)
        {
            BufferProxy<HBuffer>::UnmapBuffer(currentlyMappedBuffer->buffer);
            usedBuffers.push_back(currentlyMappedBuffer);
            currentlyMappedData = nullptr;
            currentlyMappedBuffer = nullptr;
            currentlyUsedSize = 0;
        }
    }

private:
    BufferInfo* currentlyMappedBuffer = nullptr;
    uint8* currentlyMappedData = nullptr;
    uint32 currentlyUsedSize = 0;
    List<BufferInfo*> freeBuffers;
    List<BufferInfo*> usedBuffers;
    List<BufferInfo*> buffersToUnmap;
};

BufferAllocator<rhi::HVertexBuffer> vertexBufferAllocator;
BufferAllocator<rhi::HIndexBuffer> indexBufferAllocator;

rhi::HIndexBuffer currQuadList;
uint32 currMaxQuadCount = 0;
bool quadListRestoreRegistered = false;
}

AllocResultVB AllocateVertexBuffer(uint32 vertexSize, uint32 vertexCount)
{
    BufferAllocator<rhi::HVertexBuffer>::BufferAllocateResult result = vertexBufferAllocator.AllocateData(vertexSize, vertexCount);
    return AllocResultVB{ result.buffer, result.data, result.base, result.count };
}

AllocResultIB AllocateIndexBuffer(uint32 indexCount)
{
    BufferAllocator<rhi::HIndexBuffer>::BufferAllocateResult result = indexBufferAllocator.AllocateData(2, indexCount);
    return AllocResultIB{ result.buffer, reinterpret_cast<uint16*>(result.data), result.base, result.count };
}

const uint32 VERTICES_PER_QUAD = 4;
const uint32 INDICES_PER_QUAD = 6;

void FillQuadListData(uint16* indices, uint32 quadCount)
{
    for (uint32 i = 0; i < quadCount; ++i)
    {
        indices[i * INDICES_PER_QUAD + 0] = i * VERTICES_PER_QUAD + 0;
        indices[i * INDICES_PER_QUAD + 1] = i * VERTICES_PER_QUAD + 1;
        indices[i * INDICES_PER_QUAD + 2] = i * VERTICES_PER_QUAD + 2;
        indices[i * INDICES_PER_QUAD + 3] = i * VERTICES_PER_QUAD + 2;
        indices[i * INDICES_PER_QUAD + 4] = i * VERTICES_PER_QUAD + 1;
        indices[i * INDICES_PER_QUAD + 5] = i * VERTICES_PER_QUAD + 3; //preserve order
    }
}

void RestoreQuadList()
{
    if (currQuadList.IsValid() && rhi::NeedRestoreIndexBuffer(currQuadList))
    {
        uint16* bufferData = new uint16[currMaxQuadCount * INDICES_PER_QUAD];
        FillQuadListData(bufferData, currMaxQuadCount);
        rhi::UpdateIndexBuffer(currQuadList, bufferData, 0, currMaxQuadCount * INDICES_PER_QUAD * sizeof(uint16));
        SafeDeleteArray(bufferData);
    }
}

rhi::HIndexBuffer AllocateQuadListIndexBuffer(uint32 quadCount)
{
    if (!quadListRestoreRegistered)
    {
        RenderCallbacks::RegisterResourceRestoreCallback(MakeFunction(&RestoreQuadList));
        quadListRestoreRegistered = true;
    }
    if (quadCount > currMaxQuadCount)
    {
        if (currQuadList.IsValid())
            rhi::DeleteIndexBuffer(currQuadList);

        currMaxQuadCount = Max(currMaxQuadCount * 2, quadCount);

        uint32 indiciesCount = currMaxQuadCount * INDICES_PER_QUAD;
        uint16* bufferData = new uint16[indiciesCount];
        FillQuadListData(bufferData, currMaxQuadCount);

        rhi::IndexBuffer::Descriptor iDesc;
        iDesc.size = indiciesCount * sizeof(uint16);
        iDesc.usage = rhi::USAGE_STATICDRAW;
        iDesc.initialData = bufferData;

        currQuadList = rhi::CreateIndexBuffer(iDesc);

        SafeDeleteArray(bufferData);
    }
    return currQuadList;
}

void BeginFrame()
{
    vertexBufferAllocator.BeginFrame();
    indexBufferAllocator.BeginFrame();
}

void EndFrame()
{
    vertexBufferAllocator.EndFrame();
    indexBufferAllocator.EndFrame();
}

void Clear()
{
    if (currQuadList.IsValid())
    {
        rhi::DeleteIndexBuffer(currQuadList);
        currQuadList = rhi::HIndexBuffer();
        currMaxQuadCount = 0;
    }
    vertexBufferAllocator.Clear();
    indexBufferAllocator.Clear();
}
void SetPageSize(uint32 size)
{
    pageSize = size;

    vertexBufferAllocator.Clear();
    indexBufferAllocator.Clear();
}
}
}
