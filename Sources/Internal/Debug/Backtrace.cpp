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

// clang-format off
#include "Debug/Backtrace.h"
#include "Utils/StringFormat.h"

#if defined(__DAVAENGINE_WIN32__)
#   include "Concurrency/Atomic.h"
#   include "Concurrency/Mutex.h"
#   include "Concurrency/LockGuard.h"
#   include <dbghelp.h>
#elif defined(__DAVAENGINE_WIN_UAP__)
#   include "Concurrency/Atomic.h"
#   include "Concurrency/Mutex.h"
#   include "Concurrency/LockGuard.h"
#   include "Utils/Utils.h"

// Types and prototypes from dbghelp.h
#define MAX_SYM_NAME            2000

typedef struct _SYMBOL_INFO {
    ULONG       SizeOfStruct;
    ULONG       TypeIndex;        // Type Index of symbol
    ULONG64     Reserved[2];
    ULONG       Index;
    ULONG       Size;
    ULONG64     ModBase;          // Base Address of module comtaining this symbol
    ULONG       Flags;
    ULONG64     Value;            // Value of symbol, ValuePresent should be 1
    ULONG64     Address;          // Address of symbol including base address of module
    ULONG       Register;         // register holding value or pointer to value
    ULONG       Scope;            // scope of the symbol
    ULONG       Tag;              // pdb classification
    ULONG       NameLen;          // Actual length of name
    ULONG       MaxNameLen;
    CHAR        Name[1];          // Name of symbol
} SYMBOL_INFO, *PSYMBOL_INFO;

BOOL (__stdcall *SymInitialize_impl)(HANDLE hProcess, PCSTR UserSearchPath, BOOL fInvadeProcess);
BOOL (__stdcall *SymFromAddr_impl)(HANDLE hProcess, DWORD64 Address, PDWORD64 Displacement, PSYMBOL_INFO Symbol);
BOOL (__stdcall *SymGetSearchPath_impl)(HANDLE hProcess, PSTR SearchPath, DWORD SearchPathLength);

// Wrapper function to use same code for win32 and winuap
BOOL SymInitialize(HANDLE hProcess, PCSTR UserSearchPath, BOOL fInvadeProcess)
{
    if (SymInitialize_impl != nullptr)
        return SymInitialize_impl(hProcess, UserSearchPath, fInvadeProcess);
    return FALSE;
}

BOOL SymFromAddr(HANDLE hProcess, DWORD64 Address, PDWORD64 Displacement, PSYMBOL_INFO Symbol)
{
    if (SymFromAddr_impl != nullptr)
        return SymFromAddr_impl(hProcess, Address, Displacement, Symbol);
    return FALSE;
}

BOOL SymGetSearchPath(HANDLE hProcess, PSTR SearchPath, DWORD SearchPathLength)
{
    if (SymGetSearchPath_impl != nullptr)
        return SymGetSearchPath_impl(hProcess, SearchPath, SearchPathLength);
    return FALSE;
}

#elif defined(__DAVAENGINE_APPLE__)
#   include <execinfo.h>
#   include <dlfcn.h>
#   include <cxxabi.h>
#elif defined(__DAVAENGINE_ANDROID__)
#   include <dlfcn.h>
#   include <cxxabi.h>
#   include <unwind.h>
#include <android/log.h>
#endif

namespace DAVA
{
namespace Debug
{

namespace
{

#if defined(__DAVAENGINE_WINDOWS__)
void InitSymbols()
{
    static Atomic<bool> symbolsInited = false;
    if (!symbolsInited)
    {
        // All DbgHelp functions are single threaded
        static Mutex initMutex;
        LockGuard<Mutex> lock(initMutex);
        if (!symbolsInited)
        {

#if defined(__DAVAENGINE_WIN_UAP__)
            // Step into land of black magic and fire-spitting dragons
            // as microsoft forbids direct using of dbghelp functions
            MEMORY_BASIC_INFORMATION bi;
            VirtualQuery(static_cast<void*>(&GetModuleFileNameA), &bi, sizeof(bi));
            HMODULE hkernel = reinterpret_cast<HMODULE>(bi.AllocationBase);

            HMODULE (WINAPI *LoadLibraryW)(LPCWSTR lpLibFileName);
            LoadLibraryW = reinterpret_cast<decltype(LoadLibraryW)>(GetProcAddress(hkernel, "LoadLibraryW"));

            if (LoadLibraryW != nullptr)
            {
                HMODULE hdbghelp = LoadLibraryW(L"dbghelp.dll");
                if (hdbghelp)
                {
                    SymInitialize_impl = reinterpret_cast<decltype(SymInitialize_impl)>(GetProcAddress(hdbghelp, "SymInitialize"));
                    SymFromAddr_impl = reinterpret_cast<decltype(SymFromAddr_impl)>(GetProcAddress(hdbghelp, "SymFromAddr"));
                    SymGetSearchPath_impl = reinterpret_cast<decltype(SymGetSearchPath_impl)>(GetProcAddress(hdbghelp, "SymGetSearchPath"));
                }
            }

            // Winuap application has read-only access to executable folder and read-write access to application folder
            // So to correctly show function names in callstack user must copy pdb file into one of the following locations:
            // - executable folder: you can copy pdb only when running app from IDE
            // - application folder: %LOCALAPPDATA%\Packages\[app-guid]\LocalState
            // If pdb file is placed somewhere else application will have no access to it
            // dbghelp functions are available only for desktop platfoms not mobile
            using Windows::Storage::ApplicationData;
            String path = WStringToString(ApplicationData::Current->LocalFolder->Path->Data());
            path += ";.";
            const char* symPath = path.c_str();
#else
            const char* symPath = nullptr;
#endif

            SymInitialize(GetCurrentProcess(), symPath, TRUE);
            // Do not regard return value of SymInitialize: if this call failed then next call will likely fail too
            symbolsInited = true;
        }
    }
}
#endif

#if defined(__DAVAENGINE_ANDROID__)

struct StackCrawlState
{
    size_t count;
    void** frames;
};

_Unwind_Reason_Code TraceFunction(struct _Unwind_Context* context, void* arg)
{
    StackCrawlState* state = static_cast<StackCrawlState*>(arg);
    if (state->count > 0)
    {
        uintptr_t pc = _Unwind_GetIP(context);
        if (pc != 0)
        {
            *state->frames = reinterpret_cast<void*>(pc);
            state->frames += 1;
            state->count -= 1;
            return _URC_NO_REASON;
        }
    }
    return _URC_END_OF_STACK;
}
#endif
} // anonymous namespace

DAVA_NOINLINE size_t GetStackFrames(void* frames[], size_t framesToCapture)
{
    size_t nframes = 0;
#if defined(__DAVAENGINE_WINDOWS__)
    // CaptureStackBackTrace is supported either on Win32 and WinUAP
    nframes = CaptureStackBackTrace(0, static_cast<DWORD>(framesToCapture), frames, nullptr);
#elif defined(__DAVAENGINE_APPLE__)
    nframes = backtrace(frames, static_cast<int>(framesToCapture));
#elif defined(__DAVAENGINE_ANDROID__)
    StackCrawlState state;
    state.count = framesToCapture;
    state.frames = frames;
    _Unwind_Backtrace(&TraceFunction, &state);
    nframes = framesToCapture - state.count;
#endif
    return nframes;
}

String DemangleSymbol(const char8* symbol)
{
#if defined(__DAVAENGINE_WINDOWS__)
    // On Win32 SymFromAddr returns already undecorated name
    return String(symbol);
#elif defined(__DAVAENGINE_APPLE__) || defined(__DAVAENGINE_ANDROID__)
    String result;
    char* demangled = abi::__cxa_demangle(symbol, nullptr, nullptr, nullptr);
    if (demangled != nullptr)
    {
        result = demangled;
        free(demangled);
    }
    return result;
#endif
}

String GetSymbolFromAddr(void* addr, bool demangle)
{
    String result;
#if defined(__DAVAENGINE_WINDOWS__)
    const size_t NAME_BUFFER_SIZE = MAX_SYM_NAME + sizeof(SYMBOL_INFO);
    char8 nameBuffer[NAME_BUFFER_SIZE];

    SYMBOL_INFO* symInfo = reinterpret_cast<SYMBOL_INFO*>(nameBuffer);
    symInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
    symInfo->MaxNameLen = NAME_BUFFER_SIZE - sizeof(SYMBOL_INFO);

    {
        InitSymbols();

        // All DbgHelp functions are single threaded
        static Mutex mutex;
        LockGuard<Mutex> lock(mutex);
        if (SymFromAddr(GetCurrentProcess(), reinterpret_cast<DWORD64>(addr), nullptr, symInfo))
            result = symInfo->Name;
    }

#elif defined(__DAVAENGINE_APPLE__) || defined(__DAVAENGINE_ANDROID__)
    Dl_info dlinfo;
    if (dladdr(addr, &dlinfo) != 0 && dlinfo.dli_sname != nullptr)
    {
        if (demangle)
            result = DemangleSymbol(dlinfo.dli_sname);
        if (result.empty())
            result = dlinfo.dli_sname;
    }
#endif
    return result;
}

DAVA_NOINLINE Vector<StackFrame> GetBacktrace(size_t framesToCapture)
{
    const size_t DEFAULT_SKIP_COUNT = 2;    // skip this function and GetStackFrames function
    const size_t MAX_BACKTRACE_COUNT = 64;

    framesToCapture = std::min(framesToCapture, MAX_BACKTRACE_COUNT);

    void* frames[MAX_BACKTRACE_COUNT + DEFAULT_SKIP_COUNT];
    size_t nframes = GetStackFrames(frames, framesToCapture + DEFAULT_SKIP_COUNT);

    Vector<StackFrame> backtrace;
    if (nframes > DEFAULT_SKIP_COUNT)
    {
        backtrace.reserve(nframes);
        
        // Skip irrelevant GetStackFrames and GetBacktrace functions by name as different compilers
        // can include or exclude GetStackFrames function depending on compiler wish
        // TODO: find the way to tell ios compiler not inlining GetStackFrames function (DAVA_NOINLINE does not help)
        size_t usefulFramesStart = 0;
        for (;usefulFramesStart < DEFAULT_SKIP_COUNT;++usefulFramesStart)
        {
            String s = GetSymbolFromAddr(frames[usefulFramesStart]);
            if (s.find("DAVA::Debug::GetStackFrames") == String::npos && s.find("DAVA::Debug::GetBacktrace") == String::npos)
            {
                backtrace.emplace_back(frames[usefulFramesStart], std::move(s));
                usefulFramesStart += 1;
                break;
            }
        }
        
        for (size_t i = usefulFramesStart;i < nframes;++i)
        {
            backtrace.emplace_back(frames[i], GetSymbolFromAddr(frames[i]));
        }
    }
    return backtrace;
}

String BacktraceToString(const Vector<StackFrame>& backtrace, size_t nframes)
{
    String result;
    size_t n = std::min(nframes, backtrace.size());
    for (size_t i = 0;i != n;++i)
    {
        result += Format("    #%u: %s [%p]\n", static_cast<uint32>(i), backtrace[i].function.c_str(), backtrace[i].addr);
    }
    return result;
}

String BacktraceToString(size_t framesToCapture)
{
    return BacktraceToString(GetBacktrace(framesToCapture));
}

void BacktraceToLog(const Vector<StackFrame>& backtrace, Logger::eLogLevel ll)
{
    Logger* logger = Logger::Instance();
    if (logger != nullptr)
    {
        logger->Log(ll, "==== callstack ====");
        for (size_t i = 0, n = backtrace.size();i != n;++i)
        {
            logger->Log(ll, "    #%u: %s [%p]", static_cast<uint32>(i), backtrace[i].function.c_str(), backtrace[i].addr);
        }
        logger->Log(ll, "==== callstack end ====");
    }
}

void BacktraceToLog(size_t framesToCapture, Logger::eLogLevel ll)
{
    BacktraceToLog(GetBacktrace(framesToCapture), ll);
}

} // namespace Debug
} // namespace DAVA
