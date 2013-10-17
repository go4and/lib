/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include <boost/config.hpp>

#if defined(BOOST_WINDOWS)
#include <windows.h>
#include <tlhelp32.h>
#else
#include <pthread.h>
#endif

#include "threads.hpp"

namespace mstd {

thread_id this_thread_id()
{
#if defined(BOOST_WINDOWS)
    return GetCurrentThreadId();
#elif defined(__APPLE__)
    return reinterpret_cast<thread_id>(pthread_self());
#else
    return pthread_self();
#endif
}

size_t count_process_threads()
{
#if BOOST_WINDOWS
    DWORD pid = GetCurrentProcessId();
    HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, pid);
    if(handle != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(entry);
        if(Process32First(handle, &entry))
        {
            do {
                if(pid == entry.th32ProcessID)
                    return entry.cntThreads;
            } while(Process32Next(handle, &entry));
        }
    }
#endif
    return 0;
}

}
