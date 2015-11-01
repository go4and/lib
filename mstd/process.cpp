/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include <boost/config.hpp>
#include <boost/scope_exit.hpp>

#include "filesystem.hpp"

#if BOOST_WINDOWS
#include <Windows.h>
#include <tlhelp32.h>
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
    return boost::filesystem::path(buf);
#endif
}

int execute_file(const boost::filesystem::wpath & path)
{
#if BOOST_WINDOWS
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    si.cb = sizeof(si);
    boost::filesystem::wpath parent = path;
    parent.remove_filename();
    if(CreateProcessW(NULL, const_cast<wchar_t*>(wfname(path).c_str()), NULL, NULL, true, CREATE_NO_WINDOW, NULL, wfname(parent).c_str(), &si, &pi))
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return pi.dwProcessId;
    } else
        return -1;
#else
    std::string fname = mstd::utf8fname(path);
    int result = vfork();
    if(!result)
    {
        execl(fname.c_str(), fname.c_str(), NULL);
        _exit(0);
    }
    return result;
#endif
}

#if BOOST_WINDOWS
std::wstring escape(const std::wstring & input)
{
    if(input.empty())
        return L"\"\"";
    else {
        bool quote = std::find(input.begin(), input.end(), ' ') != input.end() || std::find(input.begin(), input.end(), '\t') != input.end();

        std::wstring result;
        if(quote)
            result += L'\"';
        for(std::wstring::const_iterator i = input.begin(), end = input.end(); i != end; ++i)
        {
            if(*i == L'\"')
                result += L'\\';
            result += *i;
        }   

        if(quote)
            result += L'\"';
        return result;
    }
}
#endif

int execute_file(const boost::filesystem::wpath & path, const std::vector<std::wstring> & arguments, void ** handle)
{    
#if !BOOST_WINDOWS
    std::string fname = mstd::utf8fname(path);
    std::vector<std::string> args;
    args.reserve(arguments.size());
    std::vector<char*> argv;
    argv.reserve(arguments.size() + 2);
    argv.push_back(const_cast<char*>(fname.c_str()));
    for(std::vector<std::wstring>::const_iterator i = arguments.begin(), end = arguments.end(); i != end; ++i)
    {
        args.push_back(utf8(*i));
        argv.push_back(const_cast<char*>(args.back().c_str()));
    }
    argv.push_back(0);
    int result = vfork();
    if(!result)
    {
        execv(fname.c_str(), &argv[0]);
        _exit(0);
    }
    return result;
#else
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    HANDLE ipipe = 0, opipe = 0;
    CreatePipe(&ipipe, &opipe, 0, 0);
    memset(&si, 0, sizeof(si));
    si.hStdInput = ipipe;
    si.hStdOutput = si.hStdError = opipe;
    memset(&pi, 0, sizeof(pi));
    si.cb = sizeof(si);
    boost::filesystem::wpath parent = path;
    parent.remove_filename();
    std::wstring command = escape(wfname(path));
    for(std::vector<std::wstring>::const_iterator i = arguments.begin(), end = arguments.end(); i != end; ++i)
    {
        command += L' ';
        command += escape(*i);
    }

    if(CreateProcessW(NULL, const_cast<wchar_t*>(command.c_str()), NULL, NULL, true, 0, NULL, wfname(parent).c_str(), &si, &pi))
    {
        if(handle)
            *handle = pi.hProcess;
        else
            CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return pi.dwProcessId;
    } else {
        if(handle)
            *handle = INVALID_HANDLE_VALUE;
        return -1;
    }
#endif
}

void make_executable(const boost::filesystem::wpath & path, bool user, bool group, bool other)
{
    boost::system::error_code ec;
    boost::filesystem::perms perms = boost::filesystem::add_perms;
    if(user)
        perms |= boost::filesystem::owner_exe;
    if(group)
        perms |= boost::filesystem::group_exe;
    if(other)
        perms |= boost::filesystem::others_exe;
    permissions(path, perms, ec);
}

#if BOOST_WINDOWS
int find_parent(int pid)
{
	HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	BOOST_SCOPE_EXIT(handle)
	{
		if (handle)
			CloseHandle(handle);
	} BOOST_SCOPE_EXIT_END;
	if (handle)
	{
		PROCESSENTRY32W pe;
		memset(&pe, 0, sizeof(pe));
		pe.dwSize = sizeof(pe);

		if (Process32First(handle, &pe))
		{
			do {
				if (pe.th32ProcessID == pid)
					return pe.th32ParentProcessID;
			} while (Process32Next(handle, &pe));
		}
	}
	return 0;
}
#endif

}
