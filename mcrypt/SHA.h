#pragma once

#ifndef MCRYPT_BUILDING
#include <vector>

#include <boost/aligned_storage.hpp>
#endif

#include "Config.h"

#include "Defines.h"
#include "Hash.h"

namespace mcrypt {
    
class MCRYPT_DECL SHA1 : public HashEngine<SHA1, SHA1Digest> {
public:
    SHA1();

    using HashEngine<SHA1, SHA1Digest>::feed;
    void feed(const void * src, size_t len);

    using HashEngine<SHA1, SHA1Digest>::digest;
    void digest(result_type & out);
private:
    typedef boost::aligned_storage<96> Context;
    Context context_;
};

inline SHA1Digest sha1String(const std::string & input)
{
    return hashString<SHA1>(input);
}

inline SHA1Digest sha1Buffer(const void * data, size_t len)
{
    return hashBuffer<SHA1>(data, len);
}

std::string toBase64(const SHA1Digest & digest, bool url = false);
SHA1Digest fromBase64(const std::string & string);

class MCRYPT_DECL SHA256 : public HashEngine<SHA256, SHA256Digest> {
public:
    SHA256();

    using HashEngine<SHA256, SHA256Digest>::feed;
    void feed(const void * src, size_t len);

    using HashEngine<SHA256, SHA256Digest>::digest;
    void digest(SHA256Digest & out);
private:
    typedef boost::aligned_storage<112> Context;
    Context context_;
};

inline SHA256Digest sha256String(const std::string & input)
{
    return hashString<SHA256>(input);
}

inline SHA256Digest sha256Buffer(const void * data, size_t len)
{
    return hashBuffer<SHA256>(data, len);
}

inline SHA256Digest sha256Buffer(const char * begin, const char * end)
{
    return hashBuffer<SHA256>(begin, end - begin);
}

}
