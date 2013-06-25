/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

#include "Cipher.h"

namespace mcrypt {

MCRYPT_CIPHER(BlowfishCbc);
MCRYPT_CIPHER(BlowfishOfb);
MCRYPT_CIPHER(BlowfishCfb);

class BlowfishChunk {
public:
    explicit BlowfishChunk(const char * key, size_t keySize);

    void decryptChunk(uint32_t * chunk) const;
    void encryptChunk(uint32_t * chunk) const;
private:
    typedef boost::aligned_storage<4168> Context;
    Context context_;
};

}
