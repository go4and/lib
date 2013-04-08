#pragma once

#ifndef MCRYPT_BUILDING
#include <boost/function.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

#include <mstd/exception.hpp>
#include <mstd/pointer_cast.hpp>
#endif

#include "Config.h"

#include "Defines.h"

#include "PKey.h"

namespace mstd {
    class rc_buffer;
}

namespace mcrypt {

typedef unsigned long error_t;

enum Padding {
    pdDefault,
    pdNone,
    pdPKCS1,
    pdPKCS1_OAEP,
};

enum SignType {
    stSHA1,
    stMD5,
};

class MCRYPT_DECL RSA : public GenericPKey {
    BOOST_MOVABLE_BUT_NOT_COPYABLE(RSA);
public:
    static RSA fromPrivateKey(const char * src, size_t len, Error & error);
    inline static RSA fromPrivateKey(const unsigned char * src, size_t len, Error & error)
    {
        return fromPrivateKey(mstd::pointer_cast<const char*>(src), len, error);
    }
    inline static RSA fromPrivateKey(const std::vector<char> & src, Error & error)
    {
        return fromPrivateKey(&src[0], src.size(), error);
    }
    static RSA fromPrivateKey(const mstd::rc_buffer & src, Error & error);
    inline static RSA fromPrivateKey(const std::vector<unsigned char> & src, Error & error)
    {
        return fromPrivateKey(&src[0], src.size(), error);
    }

    static RSA fromPublicKey(const char * src, size_t len, Error & error);
    inline static RSA fromPublicKey(const unsigned char * src, size_t len, Error & error)
    {
        return fromPublicKey(mstd::pointer_cast<const char*>(src), len, error);
    }
    inline static RSA fromPublicKey(const std::vector<char> & src, Error & error)
    {
        return fromPublicKey(&src[0], src.size(), error);
    }
    inline static RSA fromPublicKey(const std::vector<unsigned char> & src, Error & error)
    {
        return fromPublicKey(&src[0], src.size(), error);
    }

    explicit RSA(void * evp = 0)
        : GenericPKey(evp)
    {
    }

    RSA(BOOST_RV_REF(RSA) rhs)
        : GenericPKey(boost::move(static_cast<GenericPKey&>(rhs)))
    {
    }

    void operator=(BOOST_RV_REF(RSA) rhs)
    {
        *static_cast<GenericPKey*>(this) = boost::move(static_cast<GenericPKey&>(rhs));
    }

    size_t availableSize(bool publicEncrypt = true, Padding padding = pdDefault) const;

    size_t publicDecrypt(char * out, const char * src, size_t len, Error & error, Padding padding = pdDefault) const;

    inline std::vector<char> publicDecrypt(const std::vector<char> & src, Error & error, Padding padding = pdDefault) const
    {
        return publicDecrypt(!src.empty() ? &src[0] : 0, src.size(), error, padding);
    }

    inline std::vector<char> publicDecrypt(const char * begin, const char * end, Error & error, Padding padding = pdDefault) const
    {
        return publicDecrypt(begin, end - begin, error, padding);
    }

    std::vector<char> publicDecrypt(const char * src, size_t len, Error & error, Padding padding = pdDefault) const
    {
        std::vector<char> result(size());
        result.resize(publicDecrypt(&result[0], src, len, error, padding));
        return result;
    }

    inline size_t privateEncrypt(char * out, const char * begin, const char * end, Error & error, Padding padding = pdDefault) const
    {
        return privateEncrypt(out, begin, end - begin, error, padding);
    }

    size_t privateEncrypt(char * out, const char * src, size_t len, Error & error, Padding padding = pdDefault) const;
};

#if 0
class MCRYPT_DECL RSA : private boost::noncopyable, public mstd::reference_counter<RSA> {
public:
    typedef boost::function<void(int, int)> GenerateListener;

    static RSAPtr generateKey(int num, unsigned long e, const GenerateListener & listener);

    static RSAPtr createFromPUBKEY(const void * buf, size_t len);
    static RSAPtr createFromNE(const unsigned char * n, size_t nlen, const unsigned char * e, size_t elen);
    
    void extractN(std::vector<char> & out);

    bool verify(SignType type, const char * message, size_t messageLen, const char * sign, size_t signLen);

    std::vector<char> extractPrivateKey() const;
    std::vector<char> extractPublicKey() const;

    ~RSA();
private:
    explicit RSA(rsa_st * rsa);

    rsa_st * impl_;
};
#endif

}
