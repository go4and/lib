#pragma once

#ifndef MCRYPT_BUILDING
#include <boost/array.hpp>
#include <boost/intrusive_ptr.hpp>
#endif

#include "Config.h"

namespace mcrypt {

typedef boost::array<unsigned char, 20> SHA1Digest;
typedef boost::array<unsigned char, 16> MD5Digest;

class RSA;

typedef boost::intrusive_ptr<RSA> RSAPtr;

}
