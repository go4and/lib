/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include "pch.h"

#include "Blowfish.h"

#include "Utils.h"

namespace mcrypt {

void bfcrypt(const std::string & password, const char * begin, const char * end, std::vector<char> & output)
{
    output.resize(4 + (end - begin + 7) / 8 * 8);
    char * p = &output[0];
    *mstd::pointer_cast<uint32_t*>(p) = static_cast<uint32_t>(end - begin);

    BlowfishCbc bf(_encrypt=true, _key=password, _padding=false);
    bf.process(p + 4, begin, end);
}

void bfdecrypt(const std::string & password, const char * begin, const char * end, std::vector<char> & output)
{
    size_t len = *mstd::pointer_cast<const boost::uint32_t*>(begin);
    output.resize((len + 7) / 8 * 8);
    if(len)
    {
        BlowfishCbc bf(_decrypt=true, _key=password, _padding=false);
        bf.process(&output[0], begin + 4, end);
    }
    output.resize(len);
}

}
