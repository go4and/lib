#pragma once

#if !defined(MCRYPT_BUILDING)
#include <boost/move/move.hpp>
#endif

namespace mstd {
    class rc_buffer;
}

namespace mcrypt {

class Error;

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

    mstd::rc_buffer publicPem() const;
    mstd::rc_buffer privatePem() const;
    mstd::rc_buffer publicDer() const;
    mstd::rc_buffer privateDer() const;
private:
    void * key_;
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

/*class PKeyVerify : public PKeyContext {
public:
    explicit PKeyVerify(const GenericPKey & key);
    
    std::vector<char> operator()(const std::vector<char> & input) const;
private:
};*/

template<class Derived>
class PKeyContextBase : public PKeyContext {
public:
    inline size_t operator()(char * out, const char * begin, const char * end, Error & error) { return self()(out, begin, end - begin, error); }
protected:
    explicit PKeyContextBase(const GenericPKey & key)
        : PKeyContext(key)
    {
    }
private:
    Derived & self() { return *static_cast<Derived*>(this); }
};

class PKeyEncrypt : public PKeyContextBase<PKeyEncrypt> {
public:
    explicit PKeyEncrypt(const GenericPKey & key, Error & error);

    using PKeyContextBase<PKeyEncrypt>::operator();
    size_t operator()(char * out, const char * begin, size_t len, Error & error);
};

class PKeyDecrypt : public PKeyContextBase<PKeyDecrypt> {
public:
    explicit PKeyDecrypt(const GenericPKey & key, Error & error);

    using PKeyContextBase<PKeyDecrypt>::operator();
    size_t operator()(char * out, const char * begin, size_t len, Error & error);
};

class PKeyDerive : public PKeyContext {
public:
    explicit PKeyDerive(const GenericPKey & key, const GenericPKey & peer, Error & error);

    size_t operator()(char * out, size_t outlen, Error & error);
    inline size_t operator()(unsigned char * out, size_t outlen, Error & error) { return (*this)(mstd::pointer_cast<char*>(out), outlen, error); }
};

}
