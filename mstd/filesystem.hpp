#pragma once

#include <boost/algorithm/string/predicate.hpp>

#include <boost/filesystem/operations.hpp>

#include "strings.hpp"

namespace mstd {

template<class Path>
Path relative_path(const Path & dir, const Path & path)
{
    typename Path::iterator i = path.begin(), j = dir.begin();
    while(i != path.end() && j != dir.end() && boost::iequals(i->native(), j->native()))
    {
        ++i;
        ++j;
    }
    if(i == path.begin())
        return path;
    else {
        Path result;
        for(; j != dir.end(); ++j)
            result /= L"..";
        for(; i != path.end(); ++i)
            result /= *i;
        return result;
    }
}

template<class Path>
void create_directories(const Path & path)
{
    Path dummy;
    for(typename Path::iterator i = path.begin(), end = path.end(); i != end; ++i)
    {
        dummy /= *i;
        if(!exists(dummy))
            create_directory(dummy);
    }
}

inline std::string utf8fname(const boost::filesystem::wpath & path)
{
#if BOOST_WINDOWS
#if BOOST_FILESYSTEM_VERSION >= 3
	return utf8(path.native());
#else
    return utf8(path.external_file_string());
#endif
#else
#if BOOST_FILESYSTEM_VERSION >= 3
	return path.native();
#else
    return path.external_file_string();
#endif
#endif
}

inline std::wstring wfname(const boost::filesystem::wpath & path)
{
#if BOOST_WINDOWS
#if BOOST_FILESYSTEM_VERSION >= 3
    return path.native();
#else
    return path.external_file_string();
#endif
#else
#if BOOST_FILESYSTEM_VERSION >= 3
    return deutf8(path.native());
#else
    return deutf8(path.external_file_string());
#endif
#endif
}

inline std::string apifname(const boost::filesystem::wpath & path)
{
#if BOOST_WINDOWS
	return narrow(wfname(path));
#else
	return utf8fname(path);
#endif
}

FILE * wfopen(const boost::filesystem::wpath & path, const char * mode);
std::streamsize file_size(FILE * file);

class rc_buffer;
rc_buffer load_file(const boost::filesystem::wpath & path, bool addZero = false);

boost::filesystem::path expand_env_vars(const std::wstring & input);
boost::filesystem::path expand_env_vars(const std::string & input);

}
