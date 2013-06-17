#pragma once

#if !defined(MCRYPT_BUILDING)
#include <boost/move/move.hpp>

#include <mstd/enum_utils.hpp>
#endif

namespace mstd {
    class rc_buffer;
}

namespace mcrypt {

class Error;

MSTD_DEFINE_ENUM_EX(KeyType, kt, (None)(RSA)(DSA)(DH)(EC)(Unknown));

class GenericPKey {
    BOOST_MOVABLE_BUT_NOT_COPYABLE(GenericPKey);
public:
    static GenericPKey fromPrivatePem(const char * buffer, size_t len, Error & error);
    static GenericPKey fromPrivatePem(const mstd::rc_buffer & data, Error & error);
    static GenericPKey fromPrivateDer(const char * buffer, size_t len, Error & error);

    static GenericPKey fromPublicPem(const char * buffer, size_t len, Error & error);
    static GenericPKey fromPublicDer(const char * buffer, size_t len, Error & error);
    static inline GenericPKey fromPublicDer(const unsigned char * buffer, size_t len, Error & error) { return fromPublicDer(mstd::pointer_cast<const char*>(buffer), len, error); }

    explicit GenericPKey(void * evp = 0)
        : key_(evp)
    {
    }

    GenericPKey(BOOST_RV_REF(GenericPKey) rhs)
        : key_(rhs.key_)
    {
        rhs.key_ = 0;
    }

    void operator=(BOOST_RV_REF(GenericPKey) rhs)
    {
        reset();
        key_ = rhs.key_;
        rhs.key_ = 0;
    }

    ~GenericPKey()
    {
        reset();
    }

    void reset();

    int size() const;
    bool operator!() const { return !key_; }
    
    void * handle() const { return key_; }

    KeyType type() const;

    mstd::rc_buffer publicPem() const;
    mstd::rc_buffer privatePem() const;
    mstd::rc_buffer publicDer() const;
    mstd::rc_buffer privateDer() const;
private:
    void * key_;
};

enum Padding {
    pdDefault,
    pdNone,
    pdPKCS1,
    pdPKCS1_OAEP,
};

class PKeyContext {
    BOOST_MOVABLE_BUT_NOT_COPYABLE(PKeyContext);
public:
    void * handle() { return context_; }
protected:
    explicit PKeyContext(const GenericPKey & key);

    PKeyContext(BOOST_RV_REF(PKeyContext) rhs)
        : context_(rhs.context_)
    {
        rhs.context_ = 0;
    }

    ~PKeyContext();
private:
    void * context_;
};

class PKeyCrypt : public PKeyContext {
public:
    PKeyCrypt(const GenericPKey & key, bool encrypt, Padding value, Error & error);
};

template<class Derived>
class PKeyCryptBase : public PKeyCrypt {
public:
    inline size_t operator()(char * out, const char * begin, const char * end, Error & error) { return self()(out, begin, end - begin, error); }
protected:
    explicit PKeyCryptBase(const GenericPKey & key, bool encrypt, Padding padding, Error & error)
        : PKeyCrypt(key, encrypt, padding, error)
    {
    }
private:
    Derived & self() { return *static_cast<Derived*>(this); }
};

class PKeyEncrypt : public PKeyCryptBase<PKeyEncrypt> {
public:
    typedef PKeyCryptBase<PKeyEncrypt> Base;

    explicit PKeyEncrypt(const GenericPKey & key, Error & error)
        : Base(key, true, pdDefault, error)
    {
    }

    explicit PKeyEncrypt(const GenericPKey & key, Padding padding, Error & error)
        : Base(key, true, padding, error)
    {
    }

    using Base::operator();
    size_t operator()(char * out, const char * begin, size_t len, Error & error);
};

class PKeyDecrypt : public PKeyCryptBase<PKeyDecrypt> {
public:
    typedef PKeyCryptBase<PKeyDecrypt> Base;

    explicit PKeyDecrypt(const GenericPKey & key, Error & error)
        : Base(key, false, pdDefault, error)
    {
    }
    
    explicit PKeyDecrypt(const GenericPKey & key, Padding padding, Error & error)
        : Base(key, false, padding, error)
    {
    }

    using Base::operator();
    size_t operator()(char * out, const char * begin, size_t len, Error & error);
};

class PKeyDerive : public PKeyContext {
public:
    explicit PKeyDerive(const GenericPKey & key, const GenericPKey & peer, Error & error);

    size_t operator()(char * out, size_t outlen, Error & error);
    inline size_t operator()(unsigned char * out, size_t outlen, Error & error) { return (*this)(mstd::pointer_cast<char*>(out), outlen, error); }
};

class HasherDescriptor;

class PKeyVerify : public PKeyContext {
public:
    explicit PKeyVerify(const GenericPKey & key, const HasherDescriptor & hasher, Padding padding, Error & error) : PKeyContext(key){ init(key, hasher, padding, error); }
    explicit PKeyVerify(const GenericPKey & key, const HasherDescriptor & hasher, Error & error) : PKeyContext(key) { init(key, hasher, pdDefault, error); }

    bool operator()(const char * input, size_t inlen, const char * sign, size_t signlen, Error & error);
    inline bool operator()(const unsigned char * input, size_t inlen, const unsigned char * sign, size_t signlen, Error & error) { return (*this)(mstd::pointer_cast<const char*>(input), inlen, mstd::pointer_cast<const char*>(sign), signlen, error); }
private:
    void init(const GenericPKey & key, const HasherDescriptor & hasher, Padding padding, Error & error);
    const void * hasher_;
};

}
