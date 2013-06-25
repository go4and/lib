/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
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

inline
#if BOOST_WINDOWS
std::string
#else
const std::string&
#endif
utf8fname(const boost::filesystem::wpath & path)
{
#if BOOST_WINDOWS
	return utf8(path.native());
#else
	return path.native();
#endif
}

inline
#if BOOST_WINDOWS
const std::wstring &
#else
std::wstring
#endif
wfname(const boost::filesystem::wpath & path)
{
#if BOOST_WINDOWS
    return path.native();
#else
    return deutf8(path.native());
#endif
}

FILE * wfopen(const boost::filesystem::wpath & path, const char * mode);
FILE * wfopen_append(const boost::filesystem::wpath & path);
std::streamsize file_size(FILE * file);

class rc_buffer;
rc_buffer load_file(const boost::filesystem::wpath & path, bool addZero = false);
bool save_file(const boost::filesystem::wpath & path, const rc_buffer & data, bool trimZero = false);

boost::filesystem::path expand_env_vars(const std::wstring & input);
boost::filesystem::path expand_env_vars(const std::string & input);

}
