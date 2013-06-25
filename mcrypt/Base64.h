/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#ifndef MCRYPT_BUILDING
#include <string>
#include <vector>

#include <boost/assert.hpp>
#endif

#include "Config.h"

namespace mcrypt {

MCRYPT_DECL std::string base64(const void * buf, size_t len, bool url = false);
inline std::string base64(const std::vector<char> & buf, bool url = false)
{
    size_t len = buf.size();
    return base64(len ? &buf[0] : 0, len, url);
}

MCRYPT_DECL std::wstring wbase64(const void * buf, size_t len, bool url = false);
inline std::wstring wbase64(const std::vector<char> & buf, bool url = false)
{
    size_t len = buf.size();
    return wbase64(len ? &buf[0] : 0, len, url);
}

MCRYPT_DECL size_t debase64(const char * str, size_t len, void * rout);

inline size_t debase64length(const char * str, size_t length)
{
    BOOST_ASSERT((length & 3) == 0);
    size_t result = length / 4 * 3;
    if(length && str[length - 1] == '=')
    {
        --result;
        if(str[length - 2] == '=')
            --result;
    }
    return result;
}

inline size_t debase64length(const std::string & str)
{
    return debase64length(str.c_str(), str.length());
}

inline size_t debase64(const std::string & src, void * out)
{
    return debase64(src.c_str(), src.length(), out);
}

MCRYPT_DECL size_t debase64(const wchar_t * str, size_t len, void * rout);

inline size_t debase64length(const wchar_t * str, size_t length)
{
    BOOST_ASSERT((length & 3) == 0);
    size_t result = length / 4 * 3;
    if(length && str[length - 1] == '=')
    {
        --result;
        if(str[length - 2] == '=')
            --result;
    }
    return result;
}

inline size_t debase64length(const std::wstring & str)
{
    return debase64length(str.c_str(), str.length());
}

inline size_t debase64(const std::wstring & src, void * out)
{
    return debase64(src.c_str(), src.length(), out);
}

inline std::vector<char> debase64(const std::string & src)
{
    std::vector<char> result(debase64length(src));
    debase64(src, &result[0]);
    return result;
}

inline std::vector<char> debase64(const std::wstring & src)
{
    std::vector<char> result(debase64length(src));
    debase64(src, &result[0]);
    return result;
}

}
