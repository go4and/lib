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
#include <boost/array.hpp>
#include <boost/intrusive_ptr.hpp>
#endif

#include "Config.h"

namespace mcrypt {

typedef boost::array<unsigned char, 20> SHA1Digest;
typedef boost::array<unsigned char, 16> MD5Digest;
typedef boost::array<unsigned char, 32> SHA256Digest;

class RSA;

}
