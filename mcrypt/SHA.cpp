#include "pch.h"

#include "Base64.h"

#include "SHA.h"

namespace mcrypt {

BOOST_STATIC_ASSERT(SHA_DIGEST_LENGTH == SHA1Digest::static_size);

SHA1::SHA1()
{
    SHA1_Init(&context_);
}

SHA1::~SHA1()
{}

void SHA1::feed(const void * src, size_t len)
{
    SHA1_Update(&context_, src, len);
}

void SHA1::feed(const std::vector<unsigned char> & src)
{
    SHA1_Update(&context_, &src[0], src.size());
}

void SHA1::feed(const std::vector<char> & src)
{
    feed(&src[0], src.size());
}

SHA1Digest SHA1::digest()
{
    SHA1Digest result;
    SHA1_Final(result.c_array(), &context_);
    return result;
}

void SHA1::digest(SHA1Digest & out)
{
    SHA1_Final(out.c_array(), &context_);
}

//////////////////////////////////////////////////////////////////////////
// Utility functions
//////////////////////////////////////////////////////////////////////////

namespace {

const size_t base64length = (SHA1Digest::static_size + 2) / 3 * 4;

}

std::string toBase64(const SHA1Digest & digest, bool url)
{
    std::string result = base64(&digest[0], digest.size(), url);
    BOOST_ASSERT(result[result.length() - 1] == '=');
    result.resize(result.length() - 1);
    return result;
}

SHA1Digest fromBase64(const std::string & string)
{
    BOOST_ASSERT(string.length() == base64length - 1);
    char temp[base64length];
    memcpy(temp, string.c_str(), base64length - 1);
    temp[base64length - 1] = '=';
    SHA1Digest result;
    size_t len = debase64(temp, base64length, &result[0]);
    (void)len;
    BOOST_ASSERT(len == result.size());
    return result;
}

}
