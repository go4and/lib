#pragma once

#ifndef MCRYPT_BUILDING
#include <boost/function.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

#include <mstd/exception.hpp>
#include <mstd/reference_counter.hpp>
#endif

#include "Config.h"

#include "Defines.h"

struct rsa_st;

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

class MCRYPT_DECL RSA : private boost::noncopyable, public mstd::reference_counter<RSA> {
public:
    typedef boost::function<void(int, int)> GenerateListener;

    static RSAPtr generateKey(int num, unsigned long e, const GenerateListener & listener);
    static RSAPtr createFromPublicKey(const std::vector<char> & src);
    static RSAPtr createFromPrivateKey(const std::vector<char> & src);

    static RSAPtr createFromPrivatePem(const void * buffer, size_t len);
    static RSAPtr createFromPublicPem(const void * buffer, size_t len);
    static RSAPtr createFromPUBKEY(const void * buf, size_t len);
    static RSAPtr createFromNE(const unsigned char * n, size_t nlen, const unsigned char * e, size_t elen);
    
    int size() const;
    int availableSize(bool publicEncrypt = true, Padding padding = pdDefault) const;

    void extractN(std::vector<char> & out);
    
    inline std::vector<char> publicEncrypt(const std::vector<char> & src, Padding padding = pdDefault) const
    {
        return publicEncrypt(!src.empty() ? &src[0] : 0, src.size(), padding);
    }

    inline std::vector<char> publicDecrypt(const std::vector<char> & src, Padding padding = pdDefault) const
    {
        return publicDecrypt(!src.empty() ? &src[0] : 0, src.size(), padding);
    }

    inline std::vector<char> privateEncrypt(const std::vector<char> & src, Padding padding = pdDefault) const
    {
        return privateEncrypt(!src.empty() ? &src[0] : 0, src.size(), padding);
    }

    inline std::vector<char> privateDecrypt(const std::vector<char> & src, Padding padding = pdDefault) const
    {
        return privateDecrypt(!src.empty() ? &src[0] : 0, src.size(), padding);
    }

    inline std::vector<char> publicEncrypt(const char * begin, const char * end, Padding padding = pdDefault) const
    {
        return publicEncrypt(begin, end - begin, padding);
    }

    inline std::vector<char> publicDecrypt(const char * begin, const char * end, Padding padding = pdDefault) const
    {
        return publicDecrypt(begin, end - begin, padding);
    }

    inline std::vector<char> privateEncrypt(const char * begin, const char * end, Padding padding = pdDefault) const
    {
        return privateEncrypt(begin, end - begin, padding);
    }

    inline size_t privateEncrypt(const char * begin, const char * end, char * out, Padding padding = pdDefault) const
    {
        return privateEncrypt(begin, end - begin, out, padding);
    }

    inline std::vector<char> privateDecrypt(const char * begin, const char * end, Padding padding = pdDefault) const
    {
        return privateDecrypt(begin, end - begin, padding);
    }

    inline size_t privateDecrypt(const char * begin, const char * end, char * out, Padding padding = pdDefault) const
    {
        return privateDecrypt(begin, end - begin, out, padding);
    }

    std::vector<char> publicEncrypt(const char * src, size_t len, Padding padding = pdDefault) const;
    std::vector<char> publicDecrypt(const char * src, size_t len, Padding padding = pdDefault) const;
    std::vector<char> privateEncrypt(const char * src, size_t len, Padding padding = pdDefault) const;
    std::vector<char> privateDecrypt(const char * src, size_t len, Padding padding = pdDefault) const;

    inline size_t publicEncrypt(const char * begin, const char * end, char * out, Padding padding = pdDefault) const
    {
        return publicEncrypt(begin, end - begin, out, padding);
    }
    
    size_t publicEncrypt(const char * src, size_t len, char * out, Padding padding = pdDefault) const;
    size_t publicDecrypt(const char * src, size_t len, char * out, Padding padding = pdDefault) const;
    size_t privateDecrypt(const char * src, size_t len, char * out, Padding padding = pdDefault) const;
    size_t privateEncrypt(const char * src, size_t len, char * out, Padding padding = pdDefault) const;

    inline std::vector<char> publicEncryptEx(const std::vector<char> & src, Padding padding = pdDefault) const
    {
        return publicEncryptEx(!src.empty() ? &src[0] : 0, src.size(), padding);
    }

    inline std::vector<char> publicDecryptEx(const std::vector<char> & src, Padding padding = pdDefault) const
    {
        return publicDecryptEx(!src.empty() ? &src[0] : 0, src.size(), padding);
    }

    inline std::vector<char> privateEncryptEx(const std::vector<char> & src, Padding padding = pdDefault) const
    {
        return privateEncryptEx(!src.empty() ? &src[0] : 0, src.size(), padding);
    }

    inline std::vector<char> privateDecryptEx(const std::vector<char> & src, Padding padding = pdDefault) const
    {
        return privateDecryptEx(!src.empty() ? &src[0] : 0, src.size(), padding);
    }

    inline std::vector<char> publicEncryptEx(const char * begin, const char * end, Padding padding = pdDefault) const
    {
        return publicEncryptEx(begin, end - begin, padding);
    }

    inline std::vector<char> publicDecryptEx(const char * begin, const char * end, Padding padding = pdDefault) const
    {
        return publicDecryptEx(begin, end - begin, padding);
    }

    inline std::vector<char> privateEncryptEx(const char * begin, const char * end, Padding padding = pdDefault) const
    {
        return privateEncryptEx(begin, end - begin, padding);
    }

    inline std::vector<char> privateDecryptEx(const char * begin, const char * end, Padding padding = pdDefault) const
    {
        return privateDecryptEx(begin, end - begin, padding);
    }

    std::vector<char> publicEncryptEx(const char * src, size_t len, Padding padding = pdDefault) const;
    std::vector<char> publicDecryptEx(const char * src, size_t len, Padding padding = pdDefault) const;
    std::vector<char> privateEncryptEx(const char * src, size_t len, Padding padding = pdDefault) const;
    std::vector<char> privateDecryptEx(const char * src, size_t len, Padding padding = pdDefault) const;

    bool verify(SignType type, const char * message, size_t messageLen, const char * sign, size_t signLen);

    std::vector<char> extractPrivateKey() const;
    std::vector<char> extractPublicKey() const;

    ~RSA();
private:
    explicit RSA(rsa_st * rsa);

    rsa_st * impl_;
};

class MCRYPT_DECL RSAException : public std::exception {
public:
    RSAException(error_t error);
    ~RSAException() throw () {}

    const char * what() const throw ();
    
    error_t error() const;
private:
    std::string what_;
    error_t error_;
};

}
