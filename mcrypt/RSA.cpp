/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include "pch.h"

#include "Error.h"

#include "RSA.h"

namespace mcrypt {

int getRsaPadding(Padding padding, bool publicEncrypt)
{
    switch(padding) {
    case pdNone:
        return RSA_NO_PADDING;
    case pdDefault:
        return publicEncrypt ? RSA_PKCS1_OAEP_PADDING : RSA_PKCS1_PADDING;
    case pdPKCS1:
        return RSA_PKCS1_PADDING;
    case pdPKCS1_OAEP:
        return RSA_PKCS1_OAEP_PADDING;
    }
    BOOST_ASSERT(false);
    return RSA_NO_PADDING;
}

//////////////////////////////////////////////////////////////////////////
// class BNHolder
//////////////////////////////////////////////////////////////////////////

class BNTraits {
public:
    static BIGNUM * null() { return NULL; }
    static void close(BIGNUM * num) { BN_free(num); }
};

typedef mstd::handle_base<BIGNUM*, mstd::comparable_traits<BNTraits> > BNHolder;

//////////////////////////////////////////////////////////////////////////
// class RSA
//////////////////////////////////////////////////////////////////////////

namespace {

template<class Func>
inline std::vector<char> process(Func func, const char * src, size_t len, ::RSA * rsa, int padding, Error & error)
{
    std::vector<char> result(RSA_size(rsa));
    int sz = func(static_cast<int>(len), mstd::pointer_cast<const unsigned char*>(src), mstd::pointer_cast<unsigned char*>(&result[0]), rsa, padding);
    if(error.checkError(sz))
        return std::vector<char>();
    result.resize(sz);
    return result;
}

template<class Func>
inline size_t process(Func func, const char * src, size_t len, char * out, ::RSA * rsa, int padding, Error & error)
{
    int sz = func(static_cast<int>(len), mstd::pointer_cast<const unsigned char*>(src), mstd::pointer_cast<unsigned char*>(out), rsa, padding);
    if(error.checkError(sz))
        return 0;
    return sz;
}

BIGNUM * extractBignum(const char *& i, const char * end, Error & error)
{
    if(end - i < 4 && error.checkResult(0))
        return 0;
    int32_t len;
    memcpy(&len, i, sizeof(len));
    len = mstd::ntoh(len);
    i += 4;
    if(end - i < len && error.checkResult(0))
        return 0;
    BIGNUM * result = BN_bin2bn(mstd::pointer_cast<const unsigned char*>(i), len, NULL);
    i += len;
    return result;
}

size_t getPaddingTail(int padding)
{
    switch(padding) {
    case RSA_NO_PADDING:
        return 0;
    case RSA_PKCS1_PADDING:
        return RSA_PKCS1_PADDING_SIZE;
    case RSA_PKCS1_OAEP_PADDING:
        return 42;
    default:
        BOOST_ASSERT(false);
        return 0;
    };
};

}

size_t RSA::publicDecrypt(char * out, const char * src, size_t len, Error & error, Padding padding) const
{
    ::RSA * impl = static_cast<EVP_PKEY*>(handle())->pkey.rsa;
    return process(&RSA_public_decrypt, src, len, out, impl, getRsaPadding(padding, false), error);
}

size_t RSA::privateEncrypt(char * out, const char * src, size_t len, Error & error, Padding padding) const
{
    ::RSA * impl = static_cast<EVP_PKEY*>(handle())->pkey.rsa;
    return process(&RSA_private_encrypt, src, len, out, impl, getRsaPadding(padding, false), error);
}

RSA RSA::fromPrivateKey(const char * src, size_t len, Error & error)
{
    const char * i = src, * end = src + len;
    BNHolder n(extractBignum(i, end, error));
    if(!n)
        return RSA(0);
    BNHolder e(extractBignum(i, end, error));
    if(!e)
        return RSA(0);
    BNHolder d(extractBignum(i, end, error));
    if(!d)
        return RSA(0);
    BNHolder p(extractBignum(i, end, error));
    if(!p)
        return RSA(0);
    BNHolder q(extractBignum(i, end, error));
    if(!q)
        return RSA(0);
    BNHolder dmp1(extractBignum(i, end, error));
    if(!dmp1)
        return RSA(0);
    BNHolder dmq1(extractBignum(i, end, error));
    if(!dmq1)
        return RSA(0);
    BNHolder iqmp(extractBignum(i, end, error));
    if(!iqmp)
        return RSA(0);

    ::RSA * impl = RSA_new();
    if(!impl && error.checkResult(0))
        return RSA(0);

    BOOST_SCOPE_EXIT((&impl)) {
        if(impl)
            RSA_free(impl);
    } BOOST_SCOPE_EXIT_END;

    impl->n = n.release();
    impl->e = e.release();
    impl->d = d.release();
    impl->p = p.release();
    impl->q = q.release();
    impl->dmp1 = dmp1.release();
    impl->dmq1 = dmq1.release();
    impl->iqmp = iqmp.release();
        
    int code = RSA_check_key(impl);
    
    if(error.checkResult(code))
        return RSA(0);
    
    EVP_PKEY * key = EVP_PKEY_new();
    EVP_PKEY_assign_RSA(key, impl);
    impl = 0;
    return RSA(key);
}

RSA RSA::fromPrivateKey(const mstd::rc_buffer & src, Error & error)
{
    return fromPrivateKey(src.data(), src.size(), error);
}

namespace {

RSA makeRSA(::RSA *& impl)
{
    EVP_PKEY * key = EVP_PKEY_new();
    EVP_PKEY_assign_RSA(key, impl);
    impl = 0;
    return RSA(key);
}

RSA makeRSA(BNHolder & n, BNHolder & e, Error & error)
{
    ::RSA * impl = RSA_new();
    if(!impl && error.checkResult(0))
        return RSA(0);
    BOOST_SCOPE_EXIT((&impl)) {
        if(impl)
            RSA_free(impl);
    } BOOST_SCOPE_EXIT_END;

    impl->n = n.release();
    impl->e = e.release();

    return makeRSA(impl);
}

}

RSA RSA::fromPublicKey(const char * src, size_t len, Error & error)
{
    const char * i = src, * end = src + len;
    BNHolder n(extractBignum(i, end, error));
    if(!n)
        return RSA(0);
    BNHolder e(extractBignum(i, end, error));
    if(!e)
        return RSA(0);

    return makeRSA(n, e, error);
}

RSA RSA::fromPUBKEY(const char * buf, size_t len, Error & error)
{
    BIO * bmem = BIO_new_mem_buf(const_cast<void*>(static_cast<const void*>(buf)), static_cast<int>(len));
    ::RSA * rsa = d2i_RSA_PUBKEY_bio(bmem, 0);
    BIO_free_all(bmem);

    if(!rsa && error.checkResult(0))
        return RSA(0);
    return makeRSA(rsa);
}

RSA RSA::fromNE(const unsigned char * n, size_t nlen, const unsigned char * e, size_t elen, Error & error)
{
    BNHolder bn(BN_bin2bn(n, static_cast<int>(nlen), 0));
    BNHolder be(BN_bin2bn(e, static_cast<int>(elen), 0));

    return makeRSA(bn, be, error);
}

size_t RSA::availableSize(bool publicEncrypt, Padding padding) const
{
    int realPadding = getRsaPadding(padding, publicEncrypt);
    return size() - getPaddingTail(realPadding);
}

RSA RSA::generateKey(int num, unsigned long e, Error & error)
{
    ::RSA * impl = RSA_generate_key(num, e, 0, 0);
    if(!impl && error.checkResult(0))
        return RSA(0);
    BOOST_SCOPE_EXIT((&impl)) {
        if(impl)
            RSA_free(impl);
    } BOOST_SCOPE_EXIT_END;

    return makeRSA(impl);
}

void RSA::extractN(std::vector<char> & out)
{
    ::RSA * impl = static_cast<EVP_PKEY*>(handle())->pkey.rsa;
    out.resize(BN_num_bytes(impl->n));
    BN_bn2bin(impl->n, mstd::pointer_cast<unsigned char*>(&out[0]));
}

}
