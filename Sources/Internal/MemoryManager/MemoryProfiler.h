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


#ifndef __DAVAENGINE_MEMORYPROFILER_H__
#define __DAVAENGINE_MEMORYPROFILER_H__

#include "Base/BaseTypes.h"

#if defined(DAVA_MEMORY_PROFILING_ENABLE)

#include "MemoryManager.h"

#define DAVA_MEMORY_PROFILER_ENABLE_LIGHTWEIGHT() DAVA::MemoryManager::Instance()->EnableLightWeightMode()
#define DAVA_MEMORY_PROFILER_UPDATE() DAVA::MemoryManager::Instance()->Update()
#define DAVA_MEMORY_PROFILER_FINISH() DAVA::MemoryManager::Instance()->Finish()

#define DAVA_MEMORY_PROFILER_REGISTER_TAG(index, name) DAVA::MemoryManager::RegisterTagName(index, name)
#define DAVA_MEMORY_PROFILER_REGISTER_ALLOC_POOL(index, name) DAVA::MemoryManager::RegisterAllocPoolName(index, name)

#define DAVA_MEMORY_PROFILER_ENTER_TAG(tag) DAVA::MemoryManager::Instance()->EnterTagScope(tag)
#define DAVA_MEMORY_PROFILER_LEAVE_TAG(tag) DAVA::MemoryManager::Instance()->LeaveTagScope(tag)

#define DAVA_MEMORY_PROFILER_GPU_ALLOC(id, size, gpuAllocPool) DAVA::MemoryManager::Instance()->TrackGpuAlloc(id, size, gpuAllocPool)
#define DAVA_MEMORY_PROFILER_GPU_DEALLOC(id, gpuAllocPool) DAVA::MemoryManager::Instance()->TrackGpuDealloc(id, gpuAllocPool)

#define DAVA_MEMORY_PROFILER_ALLOC_SCOPE(allocPool) DAVA::MemoryManager::AllocPoolScope memory_profiler_alloc_scope(allocPool)
#define DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE() DAVA::MemoryManager::AllocPoolScope memory_profiler_alloc_scope(this_class_allocation_pool)

#define DAVA_ENABLE_CLASS_ALLOCATION_TRACKING(allocPool)                                                                                \
private:                                                                                                                                \
    static const DAVA::uint32 this_class_allocation_pool = allocPool;                                                                   \
public:                                                                                                                                 \
    static void* operator new(size_t size){return DAVA::MemoryManager::Instance()->Allocate(size, this_class_allocation_pool); }       \
    static void* operator new[](size_t size) { return DAVA::MemoryManager::Instance()->Allocate(size, this_class_allocation_pool); }   \
    static void operator delete(void* ptr) DAVA_NOEXCEPT { DAVA::MemoryManager::Instance()->Deallocate(ptr); }                         \
    static void operator delete[](void* ptr)DAVA_NOEXCEPT { DAVA::MemoryManager::Instance()->Deallocate(ptr); }

#else // defined(DAVA_MEMORY_PROFILING_ENABLE)

#define DAVA_MEMORY_PROFILER_ENABLE_LIGHTWEIGHT()
#define DAVA_MEMORY_PROFILER_UPDATE()
#define DAVA_MEMORY_PROFILER_FINISH()

#define DAVA_MEMORY_PROFILER_REGISTER_TAG(index, name)
#define DAVA_MEMORY_PROFILER_REGISTER_ALLOC_POOL(index, name)

#define DAVA_MEMORY_PROFILER_ENTER_TAG(tag)
#define DAVA_MEMORY_PROFILER_LEAVE_TAG(tag)

#define DAVA_MEMORY_PROFILER_GPU_ALLOC(id, size, gpuAllocPool)
#define DAVA_MEMORY_PROFILER_GPU_DEALLOC(id, gpuAllocPool)

#define DAVA_MEMORY_PROFILER_ALLOC_SCOPE(allocPool)
#define DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE()

#define DAVA_ENABLE_CLASS_ALLOCATION_TRACKING(allocPool)

#endif // defined(DAVA_MEMORY_PROFILING_ENABLE)

#endif // __DAVAENGINE_MEMORYPROFILER_H__
