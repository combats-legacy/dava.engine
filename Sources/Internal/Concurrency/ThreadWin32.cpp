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


#include "Base/Platform.h"
#if defined(__DAVAENGINE_WINDOWS__)

#include <thread>
#include "Concurrency/Thread.h"

namespace DAVA
{

#include <windows.h>
#include <process.h>

const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push, 8)
typedef struct tagTHREADNAME_INFO
{
    DWORD dwType; // Must be 0x1000.
    LPCSTR szName; // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID
    DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

void Thread::Init()
{
}

void Thread::Shutdown()
{
    if (handle != nullptr)
    {
        Join();
        CloseHandle(handle);
        handle = nullptr;
    }
}

void Thread::Start()
{
    DVASSERT(STATE_CREATED == state);

    uintptr_t x = _beginthreadex(nullptr,
                                 static_cast<DWORD>(stackSize),
                                 &ThreadFunc,
                                 this,
                                 0,
                                 nullptr);

    if (x != 0)
    {
        handle = reinterpret_cast<HANDLE>(x);
        isJoinable.Set(true);
        state.CompareAndSwap(STATE_CREATED, STATE_RUNNING);
    }
    else
    {
        Logger::Error("Thread::Start failed to create thread: errno=%d", errno);
    }
}

unsigned __stdcall ThreadFunc(void* param)
{
#if defined(__DAVAENGINE_DEBUG__)
    /*
     inside that ifdef we set thread name through raising speciefic exception.
     https://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx
     */

    Thread* t = static_cast<Thread*>(param);

    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = t->name.c_str();
    info.dwThreadID = ::GetCurrentThreadId();
    info.dwFlags = 0;

    __try
    {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), reinterpret_cast<PULONG_PTR>(&info));
    }
    __except (EXCEPTION_CONTINUE_EXECUTION)
    {
    }
#endif

    Thread::ThreadFunction(param);
    return 0;
}

void Thread::Join()
{
    if (isJoinable.CompareAndSwap(true, false))
    {
        if (WaitForSingleObjectEx(handle, INFINITE, FALSE) != WAIT_OBJECT_0)
        {
            DAVA::Logger::Error("Thread::Join failed in WaitForSingleObjectEx: error=%u", GetLastError());
        }
    }
}

void Thread::KillNative()
{
#if defined(__DAVAENGINE_WIN_UAP__)
    DAVA::Logger::Warning("Thread::KillNative() is not implemented for Windows Store platform");
#else
    if (TerminateThread(handle, 0))
    {
        CloseHandle(handle);
        handle = nullptr;
    }
#endif
}

Thread::Id Thread::GetCurrentId()
{
    return ::GetCurrentThreadId();
}

bool DAVA::Thread::BindToProcessor(unsigned proc_n)
{
    bool success = false;

#if defined(__DAVAENGINE_WIN_UAP__)
    PROCESSOR_NUMBER proc_number{};
    proc_number.Group = 0;
    proc_number.Number = proc_n;

    success = (::SetThreadIdealProcessorEx(handle, &proc_number, nullptr) == TRUE);
#else
    DWORD_PTR mask = 1 << proc_n;
    success = (::SetThreadAffinityMask(handle, mask) == 0);
#endif

    if (!success)
    {
        Logger::FrameworkDebug("Failed bind thread to processor %d, error %d", proc_n, GetLastError());
    }

    return success;
}

void Thread::SetPriority(eThreadPriority priority)
{
    DVASSERT(state == STATE_RUNNING);
    if (threadPriority == priority)
        return;

    threadPriority = priority;
    int prio = THREAD_PRIORITY_NORMAL;
    switch (threadPriority)
    {
    case PRIORITY_LOW:
        prio = THREAD_PRIORITY_LOWEST;
        break;
    case PRIORITY_HIGH:
        prio = THREAD_PRIORITY_HIGHEST;
        break;
    }

    if (::SetThreadPriority(handle, prio) == 0)
    {
        Logger::FrameworkDebug("[Thread::SetPriority]: Cannot set thread priority");
    }
}
}

#endif
