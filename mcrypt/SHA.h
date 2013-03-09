#pragma once

#ifndef MCRYPT_BUILDING
#include <openssl/sha.h>

#include <vector>

#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>

#include <boost/filesystem/path.hpp>
#endif

#include "Config.h"

#include "Defines.h"
#include "Hash.h"

namespace mcrypt {
    
class MCRYPT_DECL SHA1 {
public:
    typedef SHA1Digest result_type;

    SHA1();
    ~SHA1();

    void feed(const void * src, size_t len);
    void feed(const std::vector<char> & src);
    void feed(const std::vector<unsigned char> & src);

    SHA1Digest digest();
    void digest(SHA1Digest & out);
private:
    SHA_CTX context_;
};

SHA1Digest sha1String(const std::string & input)
{
    return hashString<SHA1>(input);
}

SHA1Digest sha1Buffer(const void * data, size_t len)
{
    return hashBuffer<SHA1>(data, len);
}

std::string toBase64(const SHA1Digest & digest, bool url = false);
SHA1Digest fromBase64(const std::string & string);

}
