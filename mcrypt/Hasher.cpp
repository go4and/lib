#include "pch.h"

#include "Base64.h"

#include "Hasher.h"

namespace mcrypt {

GenericHasher::GenericHasher(const HasherDescriptor & descriptor)
{
    BOOST_STATIC_ASSERT(Context::size == sizeof(EVP_MD_CTX));
    const EVP_MD * md = static_cast<const EVP_MD*>(descriptor.handle());
    EVP_MD_CTX * context = static_cast<EVP_MD_CTX*>(context_.address());
    EVP_MD_CTX_init(context);
    EVP_DigestInit_ex(context, md, NULL);
}

GenericHasher::~GenericHasher()
{
    EVP_MD_CTX * context = static_cast<EVP_MD_CTX*>(context_.address());
    EVP_MD_CTX_cleanup(context);
}

void GenericHasher::update(const char * data, size_t size)
{
    EVP_MD_CTX * context = static_cast<EVP_MD_CTX*>(context_.address());
    EVP_DigestUpdate(context, data, static_cast<int>(size));
}

size_t GenericHasher::final(unsigned char * out)
{
    EVP_MD_CTX * context = static_cast<EVP_MD_CTX*>(context_.address());
    unsigned int outlen;
    EVP_DigestFinal_ex(context, out, &outlen);
    return outlen;
}

std::string hashBase64(const unsigned char * data, size_t len, bool url)
{
    std::string result = base64(data, len, url);
    len = result.length();
    while(result[len - 1] == '=')
        --len;
    result.resize(len);
    return result;
}

}
