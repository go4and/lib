#include <boost/config.hpp>

#include "filesystem.hpp"

#if BOOST_WINDOWS
#include <Windows.h>
#else

#include "strings.hpp"

#include <sys/types.h>
#include <sys/stat.h>

#if __APPLE__
#include <sys/syslimits.h>
#include <mach-o/dyld.h>
#else
#include <unistd.h>
#endif

#endif

#include "process.hpp"

namespace mstd {

boost::filesystem::wpath executable_path()
{
#if BOOST_WINDOWS
    HMODULE mh = GetModuleHandle(NULL);		
    wchar_t buffer[MAX_PATH + 2];
    GetModuleFileNameW(mh, buffer, MAX_PATH + 1);
    return boost::filesystem::wpath(buffer);
#elif __APPLE__
    char fn[PATH_MAX + 1];
    uint32_t pathlen = PATH_MAX;
    _NSGetExecutablePath(fn, &pathlen);
    boost::filesystem::wpath temp(mstd::deutf8(fn));
    boost::filesystem::wpath result;
    static const boost::filesystem::wpath dot(".");
    for(boost::filesystem::wpath::iterator i = temp.begin(), end = temp.end(); i != end; ++i)
        if(*i != dot)
            result /= *i;
    return result;
#else
    char buf[0x100 + 1];
    ssize_t size = readlink("/proc/self/exe", buf, sizeof(buf));
    buf[size] = 0;
    return boost::filesystem::wpath(mstd::deutf8(buf));
#endif
}

void execute_file(const boost::filesystem::wpath & path)
{
#if BOOST_WINDOWS
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    si.cb = sizeof(si);
    boost::filesystem::wpath parent = path;
    parent.remove_filename();
    CreateProcessW(NULL, const_cast<wchar_t*>(wfname(path).c_str()), NULL, NULL, true, 0, NULL, wfname(parent).c_str(), &si, &pi);
#else
    std::string fname = apifname(path).c_str();
    if(!vfork())
    {
        execl(fname.c_str(), NULL, NULL);
        _exit(0);
    }
#endif
}

void make_executable(const boost::filesystem::wpath & path, bool user, bool group, bool other)
{
#if !BOOST_WINDOWS
    std::string executable = mstd::apifname(path);
    struct stat st;
    if(!stat(executable.c_str(), &st))
        chmod(executable.c_str(), st.st_mode | (user ? S_IXUSR : 0) | (group ? S_IXGRP : 0) | (other ? S_IXOTH : 0));
#endif
}

}
