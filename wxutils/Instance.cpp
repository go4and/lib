/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
include "pch.h"

#include "Instance.h"

MLOG_DECLARE_LOGGER(wxutils_instance);

#if BOOST_WINDOWS
extern "C"
{
    WXDLLIMPEXP_BASE HINSTANCE wxGetInstance();
}
#endif

namespace wxutils {

#if BOOST_WINDOWS
HINSTANCE instance()
{
    return wxGetInstance();
}

namespace {

HMODULE ntdll = 0;
typedef void (_stdcall *SHAInit)(void * handle);
typedef void (_stdcall *SHAUpdate)(void * handle, const void *data, unsigned int len);
typedef void (_stdcall *SHAFinal)(void * handle, unsigned char *md);
SHAInit shaInit;
SHAUpdate shaUpdate;
SHAFinal shaFinal;

std::string getSharedName()
{
    std::string path = mstd::utf8_to_lower_copy(mstd::utf8fname(mstd::executable_path()));
    MLOG_DEBUG("getShareName for: " << path);
    if(!ntdll)
    {
        ntdll = GetModuleHandle(L"ntdll.dll");
        shaInit = mstd::pointer_cast<SHAInit>(GetProcAddress(ntdll, "A_SHAInit"));
        shaUpdate = mstd::pointer_cast<SHAUpdate>(GetProcAddress(ntdll, "A_SHAUpdate"));
        shaFinal = mstd::pointer_cast<SHAFinal>(GetProcAddress(ntdll, "A_SHAFinal"));
        MLOG_DEBUG("shaInit: " << shaInit << ", shaUpdate: " << shaUpdate << ", shaFinal: " << shaFinal);
    }
    char buffer[0x100];
    shaInit(buffer);
    shaUpdate(buffer, path.c_str(), path.size());
    unsigned char output[20];
    memset(output, 0, sizeof(output));
    shaFinal(buffer, output);
    MLOG_DEBUG("output: " << mlog::dump(output, sizeof(output)));
    std::string result;
    result.resize(sizeof(output) * 2);
    for(size_t i = 0; i != sizeof(output); ++i)
    {
        result[i * 2    ] = mstd::hex_table[output[i] >> 4];
        result[i * 2 + 1] = mstd::hex_table[output[i] & 0xf];
    }
    return result;
}

}
#endif

bool checkSingleInstance(bool show, const std::wstring & title)
{
#if BOOST_WINDOWS
    namespace bi = boost::interprocess;
    try {
        std::string sharedName = getSharedName();
        MLOG_NOTICE("checkSingleInstance for: " << sharedName);
        bi::shared_memory_object smo(bi::open_or_create, sharedName.c_str(), bi::read_write);
        bi::offset_t size;
        if(smo.get_size(size) && size != 0)
        {
            bi::mapped_region mr(smo, bi::read_only, 0, size);
            DWORD pid;
            memcpy(&pid, mr.get_address(), sizeof(pid));
            {
                HWND hwnd = ::GetTopWindow(0);
                while(hwnd)
                {
                    DWORD cpid = 0;
                    GetWindowThreadProcessId(hwnd, &cpid);
                    if(cpid == pid)
                    {
                        wchar_t buffer[0x100];
                        if(title.empty() || (GetWindowText(hwnd, buffer, sizeof(buffer) / sizeof(buffer[0])) && buffer == title))
                        {
                            if(show)
                            {
                                ShowWindow(hwnd, SW_SHOW);
                                SetForegroundWindow(hwnd);
                            }
                            return false;
                        }
                    }

                    hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
                }
            }
        }
        DWORD pid = GetCurrentProcessId();
        size = sizeof(pid);
        smo.truncate(size);
        {
            bi::mapped_region mr(smo, bi::read_write, 0, size);
            memcpy(mr.get_address(), &pid, sizeof(pid));
        }
    } catch(bi::interprocess_exception & exc) {
        MLOG_ERROR("checkSingleInstance failure: " << mstd::out_exception(exc));
    }
    return true;
#else
    return true;
#endif
}

void releaseSingleInstance()
{
#if BOOST_WINDOWS
    try {
        boost::interprocess::shared_memory_object::remove(getSharedName().c_str());
    } catch(boost::interprocess::interprocess_exception & exc) {
        MLOG_ERROR("releaseSingleInstance failure: " << mstd::out_exception(exc));
    }
#endif
}

}
