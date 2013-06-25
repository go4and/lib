/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

#ifndef MCRYPT_BUILDING
#include <vector>

#include <mstd/cstdint.hpp>
#endif

#include "Config.h"

namespace mcrypt {

typedef boost::int32_t crc32_t;

MCRYPT_DECL std::vector<char> crc32check(const std::vector<char> & src);
MCRYPT_DECL void crc32append(std::vector<char> & data);

MCRYPT_DECL crc32_t crc32(const std::vector<char> & src);
MCRYPT_DECL crc32_t crc32(const char * begin, const char * end);
MCRYPT_DECL crc32_t crc32(const std::vector<unsigned char> & src);
MCRYPT_DECL crc32_t crc32(const unsigned char * begin, const unsigned char * end);

}
