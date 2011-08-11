#pragma once

#ifndef MCRYPT_BUILDING
#include <string>
#include <vector>
#endif

#include "Config.h"

namespace mcrypt {

MCRYPT_DECL std::string base64(const char * buf, size_t len);
inline std::string base64(const std::vector<char> & buf)
{
    size_t len = buf.size();
    return base64(len ? &buf[0] : 0, len);
}
MCRYPT_DECL std::wstring wbase64(const char * buf, size_t len);
inline std::wstring wbase64(const std::vector<char> & buf)
{
    size_t len = buf.size();
    return wbase64(len ? &buf[0] : 0, len);
}

MCRYPT_DECL std::vector<char> debase64(const std::string & src);
MCRYPT_DECL std::vector<char> debase64(const std::wstring & src);

}
