/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include "pch.h"

#include "Crypt.h"

namespace mcrypt {

Crypt::Crypt(const std::vector<unsigned char> & key) : key_(key)
{
}

void Crypt::decrypt(unsigned char * i, unsigned char * end)
{
    size_t size = end - i;
    int temp = 0, j = 0;
    for(; i != end; ++i, ++j)
    {
        int temp2 = *i;
        *i = static_cast<unsigned char>(temp2 ^ key_[j & (key_.size() - 1)] ^ temp);
        temp = temp2;
    }

    // suggest little endian
    uint64_t * val = reinterpret_cast<uint64_t*>(&key_[key_.size() - 8]);
    *val += size;
}

void Crypt::encrypt(unsigned char * i, unsigned char * end)
{
    size_t size = end - i;
    int temp = 0, j = 0;
    for(; i != end; ++i, ++j)
    {
        int temp2 = *i;
        *i = temp = temp2 ^ key_[j & (key_.size() - 1)] ^ temp;
    }

    uint64_t * val = reinterpret_cast<uint64_t*>(&key_[key_.size() - 8]);
    *val += size;
}

}
