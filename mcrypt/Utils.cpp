#include "pch.h"

#include "Blowfish.h"

#include "Utils.h"

namespace mcrypt {

    namespace detail {
        const char * hexTable = "0123456789abcdef";
    }

void bfcrypt(const std::string & password, const char * begin, const char * end, std::vector<char> & output)
{
    output.resize(4 + (end - begin + 7) / 8 * 8);
    char * p = &output[0];
    *mstd::pointer_cast<uint32_t*>(p) = static_cast<uint32_t>(end - begin);

    Blowfish bf(password);
    bf.encrypt(begin, end, p + 4);
}

void bfdecrypt(const std::string & password, const char * begin, const char * end, std::vector<char> & output)
{
    size_t len = *mstd::pointer_cast<const boost::uint32_t*>(begin);
    output.resize((len + 7) / 8 * 8);
    if(len)
    {
        Blowfish bf(password);
        bf.decrypt(begin + 4, end, &output[0]);
    }
    output.resize(len);
}

}
