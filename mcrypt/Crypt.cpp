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
    size_t * val = reinterpret_cast<size_t*>(&key_[key_.size() - 8]);
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

    size_t * val = reinterpret_cast<size_t*>(&key_[key_.size() - 8]);
    *val += size;
}

}
