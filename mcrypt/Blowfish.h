#pragma once

#ifndef MCRYPT_BUILDING
#include <string>
#include <vector>

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

#include <mstd/cstdint.hpp>
#endif

#include "Cipher.h"
#include "Config.h"

namespace mcrypt {

class BlowfishBase {
public:
protected:
    void init(const char * password, size_t len, const char * ivec);

    typedef boost::aligned_storage<4 * (16 + 2 + 4 * 256)> Key;
    Key key_;
    int num_;
    unsigned char ivec_[8];
};

MCRYPT_CIPHER(BlowfishCbc, BlowfishBase);
MCRYPT_CIPHER(BlowfishOfb, BlowfishBase);
MCRYPT_CIPHER(BlowfishCfb, BlowfishBase);

}
