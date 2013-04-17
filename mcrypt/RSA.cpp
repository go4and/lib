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
    auto * impl = static_cast<EVP_PKEY*>(handle())->pkey.rsa;
    return process(&RSA_public_decrypt, src, len, out, impl, getRsaPadding(padding, false), error);
}

size_t RSA::privateEncrypt(char * out, const char * src, size_t len, Error & error, Padding padding) const
{
    auto * impl = static_cast<EVP_PKEY*>(handle())->pkey.rsa;
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
    auto * impl = static_cast<EVP_PKEY*>(handle())->pkey.rsa;
    out.resize(BN_num_bytes(impl->n));
    BN_bn2bin(impl->n, mstd::pointer_cast<unsigned char*>(&out[0]));
}

#if 0
namespace {

void invokeListener(int a, int b, void * raw)
{
    RSA::GenerateListener * listener = static_cast<RSA::GenerateListener*>(raw);
    (*listener)(a, b);
}

::RSA * extractRsa(EVP_PKEY *& key)
{
	if(!key)
        return 0;
	::RSA * rtmp = EVP_PKEY_get1_RSA(key);
	EVP_PKEY_free(key);
    key = 0;
	return rtmp;
}

}

RSAPtr RSA::createFromPublicPem(const void * buf, size_t len)
{
    BIO * bmem = BIO_new_mem_buf(const_cast<void*>(buf), static_cast<int>(len));
    EVP_PKEY * key = PEM_read_bio_PUBKEY(bmem, 0, 0, 0);
    BIO_free_all(bmem);

    if(key)
    {
        ::RSA * rsa = extractRsa(key);

        if(key)
            EVP_PKEY_free(key);
        if(rsa)
        {
            RSAPtr result(new RSA(rsa));
            return result;
        } else
            handleError();
    } else
        handleError();
    throw RSAException(0);
}

RSAPtr RSA::createFromPrivatePem(const void * buf, size_t len)
{
    BIO * bmem = BIO_new_mem_buf(const_cast<void*>(buf), static_cast<int>(len));
    EVP_PKEY * key = PEM_read_bio_PrivateKey(bmem, 0, 0, 0);
    BIO_free_all(bmem);

    if(key)
    {
        ::RSA * rsa = extractRsa(key);
        if(key)
            EVP_PKEY_free(key);
        if(rsa)
        {
            RSAPtr result(new RSA(rsa));
            return result;
        } else
            handleError();
    } else
        handleError();
    throw RSAException(0);
}

RSAPtr RSA::createFromPUBKEY(const void * buf, size_t len)
{
    BIO * bmem = BIO_new_mem_buf(const_cast<void*>(buf), static_cast<int>(len));
    ::RSA * rsa = d2i_RSA_PUBKEY_bio(bmem, 0);
    BIO_free_all(bmem);

    if(rsa)
    {
        RSAPtr result(new RSA(rsa));
        return result;
    } else
        handleError();
    throw RSAException(0);
}

RSA::RSA(::RSA * impl)
    : impl_(impl)
{
    if(!impl_)
        handleError();
}

typedef std::vector<BIGNUM*> BigNums;

std::vector<char> packBigNums(const BigNums & nums)
{
    size_t size = 0;
    for(BigNums::const_iterator i = nums.begin(); i != nums.end(); ++i)
        size += 4 + BN_num_bytes(*i);
    
    std::vector<char> result(size);
    std::vector<char>::iterator p = result.begin();
    for(BigNums::const_iterator i = nums.begin(); i != nums.end(); ++i)
    {
        BIGNUM * num = *i;
        size_t len = BN_num_bytes(num);
        *mstd::pointer_cast<boost::uint32_t*>(&*p) = htonl(static_cast<boost::uint32_t>(len));
        p += 4;
        BN_bn2bin(num, mstd::pointer_cast<unsigned char*>(&*p));
        p += len;
    }

    return result;
}

template<class Func>
static std::vector<char> process(Func func, const char * src, size_t len, ::RSA * rsa, int padding)
{
    std::vector<char> result(RSA_size(rsa));
    int sz = func(static_cast<int>(len), mstd::pointer_cast<const unsigned char*>(src), mstd::pointer_cast<unsigned char*>(&result[0]), rsa, padding);
    checkError(sz);
    result.resize(sz);
    return result;
}

template<class Func>
static size_t process(Func func, const char * src, size_t len, char * out, ::RSA * rsa, int padding)
{
    int sz = func(static_cast<int>(len), mstd::pointer_cast<const unsigned char*>(src), mstd::pointer_cast<unsigned char*>(out), rsa, padding);
    checkError(sz);
    return sz;
}

std::vector<char> RSA::publicEncrypt(const char * src, size_t len, Padding padding) const
{
    return process(&RSA_public_encrypt, src, len, impl_, getRsaPadding(padding, true));
}

std::vector<char> RSA::privateEncrypt(const char * src, size_t len, Padding padding) const
{
    return process(&RSA_private_encrypt, src, len, impl_, getRsaPadding(padding, false));
}

std::vector<char> RSA::privateDecrypt(const char * src, size_t len, Padding padding) const
{
    return process(&RSA_private_decrypt, src, len, impl_, getRsaPadding(padding, true));
}

size_t RSA::privateDecrypt(const char * src, size_t len, char * out, Padding padding) const
{
    return process(&RSA_private_decrypt, src, len, out, impl_, getRsaPadding(padding, true));
}

size_t RSA::privateEncrypt(const char * src, size_t len, char * out, Padding padding) const
{
    return process(&RSA_private_encrypt, src, len, out, impl_, getPadding(padding, false));
}

size_t RSA::publicEncrypt(const char * src, size_t len, char * out, Padding padding) const
{
    return process(&RSA_public_encrypt, src, len, out, impl_, getPadding(padding, true));
}

template<class Func>
static std::vector<char> processEx(Func func, const char * src, size_t len, ::RSA * rsa, bool encrypt, int padding)
{
    if(!len)
        return std::vector<char>();
    size_t keySize = RSA_size(rsa);
    size_t tailSize = getPaddingTail(padding);
    size_t inputBlockSize;
    size_t outputBlockSize;
    if(encrypt)
    {
        inputBlockSize = keySize - tailSize;
        outputBlockSize = keySize;
    } else {
        inputBlockSize = keySize;
        outputBlockSize = keySize - tailSize;
    }
    const char * end = src + len;
    std::vector<char> result((len + inputBlockSize - 1) / inputBlockSize * outputBlockSize);
    char * out = &result[0];
    while(src != end)
    {
        size_t clen = (src + inputBlockSize <= end ? inputBlockSize : end - src);
        int sz = func(static_cast<int>(clen), mstd::pointer_cast<const unsigned char*>(src), mstd::pointer_cast<unsigned char*>(&result[0]), rsa, padding);
        checkError(sz);
        out += sz;
        src += clen;
    }
    result.resize(out - &result[0]);
    return result;
}

std::vector<char> RSA::publicEncryptEx(const char * src, size_t len, Padding padding) const
{
    return processEx(&RSA_public_encrypt, src, len, impl_, true, getPadding(padding, true));
}

std::vector<char> RSA::publicDecryptEx(const char * src, size_t len,  Padding padding) const
{
    return processEx(&RSA_public_decrypt, src, len, impl_, false, getPadding(padding, false));
}

std::vector<char> RSA::privateEncryptEx(const char * src, size_t len, Padding padding) const
{
    return processEx(&RSA_private_encrypt, src, len, impl_, true, getPadding(padding, false));
}

std::vector<char> RSA::privateDecryptEx(const char * src, size_t len, Padding padding) const
{
    return processEx(&RSA_private_decrypt, src, len, impl_, false, getPadding(padding, true));
}

const EVP_MD * getDigest(SignType type)
{
    switch(type) {
    case stSHA1:
        return EVP_sha1();
    case stMD5:
        return EVP_md5();
    }
    BOOST_ASSERT(false);
    return 0;
}

bool RSA::verify(SignType type, const char * message, size_t messageLen, const char * sign, size_t signLen)
{
    const EVP_MD * md = getDigest(type);
    BOOST_ASSERT(md);

    EVP_MD_CTX ctx;
    memset(&ctx, 0, sizeof(ctx));
    EVP_DigestInit(&ctx, md);
	EVP_DigestUpdate(&ctx, message, messageLen);
    size_t outLen = EVP_MD_size(md);
    unsigned char * out = static_cast<unsigned char*>(alloca(outLen));
	EVP_DigestFinal(&ctx, out, 0);

    int result = RSA_verify(EVP_MD_type(md),
                            out, static_cast<int>(outLen),
                            mstd::pointer_cast<unsigned char*>(const_cast<char*>(sign)), static_cast<int>(signLen), impl_);
    return result != 0;
}

std::vector<char> RSA::extractPublicKey() const
{
    ::RSA * src = impl_;
    
    BigNums nums;
    nums.push_back(src->n);
    nums.push_back(src->e);
    
    return packBigNums(nums);
}

std::vector<char> RSA::extractPrivateKey() const
{
    ::RSA * src = impl_;
    
    BigNums nums;
    nums.push_back(src->n);
    nums.push_back(src->e);
    nums.push_back(src->d);
    nums.push_back(src->p);
    nums.push_back(src->q);
    nums.push_back(src->dmp1);
    nums.push_back(src->dmq1);
    nums.push_back(src->iqmp);
    
    return packBigNums(nums);
}
#endif

}
