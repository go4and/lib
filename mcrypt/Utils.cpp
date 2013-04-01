#include "pch.h"

#include "Blowfish.h"

#include "Utils.h"

namespace mcrypt {

void bfcrypt(const std::string & password, const char * begin, const char * end, std::vector<char> & output)
{
    output.resize(4 + (end - begin + 7) / 8 * 8);
    char * p = &output[0];
    *mstd::pointer_cast<uint32_t*>(p) = static_cast<uint32_t>(end - begin);

    BlowfishCbcEncrypt bf(password);
    bf.process(p + 4, begin, end);
}

void bfdecrypt(const std::string & password, const char * begin, const char * end, std::vector<char> & output)
{
    size_t len = *mstd::pointer_cast<const boost::uint32_t*>(begin);
    output.resize((len + 7) / 8 * 8);
    if(len)
    {
        BlowfishCbcDecrypt bf(password);
        bf.process(&output[0], begin + 4, end);
    }
    output.resize(len);
}

}
