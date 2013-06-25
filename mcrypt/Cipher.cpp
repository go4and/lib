/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include "pch.h"

#include "Cipher.h"

namespace mcrypt {

size_t CipherDescriptor::ivSize() const
{
    const EVP_CIPHER * cipher = static_cast<const EVP_CIPHER*>(evp_);
    return cipher->iv_len;
}

size_t CipherDescriptor::blockSize() const
{
    const EVP_CIPHER * cipher = static_cast<const EVP_CIPHER*>(evp_);
    return cipher->block_size;
}

size_t CipherDescriptor::keySize() const
{
    const EVP_CIPHER * cipher = static_cast<const EVP_CIPHER*>(evp_);
    return cipher->key_len;
}

namespace {

template<size_t ourSize, size_t realSize>
struct EqSize {
    BOOST_STATIC_ASSERT(ourSize == realSize);
};

}

void GenericCipher::init(const CipherDescriptor & descriptor, CipherMode mode, const char * key, size_t len, const char * ivec, bool padding)
{
    BOOST_STATIC_ASSERT((sizeof(EqSize<Context::size, sizeof(EVP_CIPHER_CTX)>)));
    const EVP_CIPHER * cipher = static_cast<const EVP_CIPHER*>(descriptor.handle());
    EVP_CIPHER_CTX * ctx = static_cast<EVP_CIPHER_CTX*>(context_.address());
    char iv[EVP_MAX_IV_LENGTH];
    if(!ivec)
    {
        memset(iv, 0, sizeof(iv));
        ivec = iv;
    }

    EVP_CIPHER_CTX_init(ctx);
    int res = EVP_CipherInit_ex(ctx, cipher, 0, 0, mstd::pointer_cast<const unsigned char *>(ivec), mode == modeEncrypt ? 1 : 0);
    BOOST_ASSERT(res && "cipher init");
    res = EVP_CIPHER_CTX_set_key_length(ctx, static_cast<int>(len));
    BOOST_ASSERT(res && "set key");
    res = ctx->cipher->init(ctx, mstd::pointer_cast<const unsigned char *>(key), mstd::pointer_cast<const unsigned char *>(ivec), mode == modeEncrypt ? 1 : 0);
    BOOST_ASSERT(res && "init cipher");
    res = EVP_CIPHER_CTX_set_padding(ctx, padding);
    BOOST_ASSERT(res && "set padding");
}

GenericCipher::~GenericCipher()
{
    EVP_CIPHER_CTX * ctx = static_cast<EVP_CIPHER_CTX*>(context_.address());
    EVP_CIPHER_CTX_cleanup(ctx);
}

size_t GenericCipher::update(char * out, const char * input, size_t len)
{
    EVP_CIPHER_CTX * ctx = static_cast<EVP_CIPHER_CTX*>(context_.address());

    int outlen;
    EVP_CipherUpdate(ctx, mstd::pointer_cast<unsigned char*>(out), &outlen, mstd::pointer_cast<const unsigned char*>(input), static_cast<int>(len));
    return outlen;
}

size_t GenericCipher::final(char * out)
{
    EVP_CIPHER_CTX * ctx = static_cast<EVP_CIPHER_CTX*>(context_.address());

    int outlen;
    EVP_CipherFinal_ex(ctx, mstd::pointer_cast<unsigned char*>(out), &outlen);
    return outlen;
}

CipherDescriptor GenericCipher::descriptor()
{
    EVP_CIPHER_CTX * ctx = static_cast<EVP_CIPHER_CTX*>(context_.address());

    return CipherDescriptor(ctx->cipher);
}

}
