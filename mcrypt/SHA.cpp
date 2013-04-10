#include "pch.h"

#include "Base64.h"

#include "SHA.h"

namespace mcrypt {

BOOST_STATIC_ASSERT(SHA_DIGEST_LENGTH == SHA1Digest::static_size);

const void * SHA1Tag::evp() { return EVP_sha1(); }

BOOST_STATIC_ASSERT(SHA256_DIGEST_LENGTH == SHA256Digest::static_size);

const void * SHA256Tag::evp() { return EVP_sha256(); }

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
