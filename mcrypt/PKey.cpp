#include "pch.h"

#include "Error.h"

#include "PKey.h"

namespace mcrypt {

int getRsaPadding(Padding padding, bool publicEncrypt);

namespace {

mstd::rc_buffer bio2rc(BIO * bmem)
{
    BUF_MEM *buf;
    BIO_get_mem_ptr(bmem, &buf);
    mstd::rc_buffer result(buf->data, buf->length);
    BIO_free_all(bmem);
    return result;
}

}

void GenericPKey::reset()
{
    if(key_)
        EVP_PKEY_free(static_cast<EVP_PKEY*>(key_));
}

int GenericPKey::size() const
{
    return EVP_PKEY_size(static_cast<EVP_PKEY*>(key_));
}

GenericPKey GenericPKey::fromPublicPem(const char * buffer, size_t len, Error & error)
{
    BIO * bmem = BIO_new_mem_buf(const_cast<char*>(buffer), static_cast<int>(len));
    EVP_PKEY * key = PEM_read_bio_PUBKEY(bmem, 0, 0, 0);
    BIO_free_all(bmem);

    if(!key && error.checkResult(0))
        return GenericPKey(0);

    return GenericPKey(key);
}

GenericPKey GenericPKey::fromPublicDer(const char * buffer, size_t len, Error & error)
{
    BIO * bmem = BIO_new_mem_buf(const_cast<char*>(buffer), static_cast<int>(len));
    EVP_PKEY * key = d2i_PUBKEY_bio(bmem, 0);
    BIO_free_all(bmem);

    if(!key && error.checkResult(0))
        return GenericPKey(0);

    return GenericPKey(key);
}

GenericPKey GenericPKey::fromPrivatePem(const mstd::rc_buffer & data, Error & error)
{
    return fromPrivatePem(data.data(), data.size(), error);
}

GenericPKey GenericPKey::fromPrivatePem(const char * buffer, size_t len, Error & error)
{
    BIO * bmem = BIO_new_mem_buf(const_cast<char*>(buffer), static_cast<int>(len));
    EVP_PKEY * key = PEM_read_bio_PrivateKey(bmem, 0, 0, 0);
    BIO_free_all(bmem);

    if(!key && error.checkResult(0))
        return GenericPKey(0);

    return GenericPKey(key);
}

GenericPKey GenericPKey::fromPrivateDer(const char * buffer, size_t len, Error & error)
{
    BIO * bmem = BIO_new_mem_buf(const_cast<char*>(buffer), static_cast<int>(len));
    EVP_PKEY * key = d2i_PrivateKey_bio(bmem, 0);
    BIO_free_all(bmem);

    if(!key && error.checkResult(0))
        return GenericPKey(0);

    return GenericPKey(key);
}

mstd::rc_buffer GenericPKey::publicPem() const
{
    BIO * bmem = BIO_new(BIO_s_mem());
    PEM_write_bio_PUBKEY(bmem, static_cast<EVP_PKEY*>(handle()));
    return bio2rc(bmem);
}

mstd::rc_buffer GenericPKey::privatePem() const
{
    BIO * bmem = BIO_new(BIO_s_mem());
    PEM_write_bio_PrivateKey(bmem, static_cast<EVP_PKEY*>(handle()), 0, 0, 0, 0, 0);
    return bio2rc(bmem);
}

mstd::rc_buffer GenericPKey::publicDer() const
{
    BIO * bmem = BIO_new(BIO_s_mem());
    i2d_PUBKEY_bio(bmem, static_cast<EVP_PKEY*>(handle()));
    return bio2rc(bmem);
}

mstd::rc_buffer GenericPKey::privateDer() const
{
    BIO * bmem = BIO_new(BIO_s_mem());
    i2d_PrivateKey_bio(bmem, static_cast<EVP_PKEY*>(handle()));
    return bio2rc(bmem);
}

PKeyContext::PKeyContext(const GenericPKey & key)
    : context_(EVP_PKEY_CTX_new(static_cast<EVP_PKEY*>(key.handle()), 0))
{
}

PKeyContext::~PKeyContext()
{
    if(context_)
        EVP_PKEY_CTX_free(static_cast<EVP_PKEY_CTX*>(context_));
}

PKeyCrypt::PKeyCrypt(const GenericPKey & key, bool encrypt, Padding padding, Error & error)
    : PKeyContext(key)
{
    EVP_PKEY_CTX * context = static_cast<EVP_PKEY_CTX*>(handle());
    int res = encrypt ? EVP_PKEY_encrypt_init(context) : EVP_PKEY_decrypt_init(context);
    if(error.checkResult(res))
        return;
    int type = EVP_PKEY_type(static_cast<EVP_PKEY*>(key.handle())->type);
    if(type == EVP_PKEY_RSA)
        EVP_PKEY_CTX_set_rsa_padding(context, getRsaPadding(padding, true));
}

size_t PKeyEncrypt::operator()(char * out, const char * begin, size_t len, Error & error)
{
    size_t outlen = std::numeric_limits<size_t>::max();
    int res = EVP_PKEY_encrypt(static_cast<EVP_PKEY_CTX*>(handle()),
                               mstd::pointer_cast<unsigned char*>(out), &outlen,
                               mstd::pointer_cast<const unsigned char*>(begin), len);
    if(error.checkResult(res))
        return 0;
    return outlen;
}

size_t PKeyDecrypt::operator()(char * out, const char * begin, size_t len, Error & error)
{
    size_t outlen = std::numeric_limits<size_t>::max();
    int res = EVP_PKEY_decrypt(static_cast<EVP_PKEY_CTX*>(handle()),
                               mstd::pointer_cast<unsigned char*>(out), &outlen,
                               mstd::pointer_cast<const unsigned char*>(begin), len);
    if(error.checkResult(res))
        return 0;
    return outlen;
}

PKeyDerive::PKeyDerive(const GenericPKey & key, const GenericPKey & peer, Error & error)
    : PKeyContext(key)
{
    EVP_PKEY_CTX * context = static_cast<EVP_PKEY_CTX*>(handle());
    int res = EVP_PKEY_derive_init(context);
    if(error.checkResult(res))
        return;
    res = EVP_PKEY_derive_set_peer(context, static_cast<EVP_PKEY*>(peer.handle()));
    if(error.checkResult(res))
        return;
}

size_t PKeyDerive::operator()(char * out, size_t outlen, Error & error)
{
    int res = EVP_PKEY_derive(static_cast<EVP_PKEY_CTX*>(handle()), mstd::pointer_cast<unsigned char*>(out), &outlen);
    if(error.checkResult(res))
        return 0;
    return outlen;
}

}
