#pragma once

#ifndef MCRYPT_BUILDING
#include <boost/aligned_storage.hpp>

#include <mstd/pointer_cast.hpp>
#endif

namespace mcrypt {

template<class Derived>
class CipherEngine {
public:
    void process(std::vector<char> & out, const std::vector<char> & src)
    {
        if(src.empty())
            out.clear();
        else {
            out.resize(src.size());
            self().process(&out[0], &src[0], src.size());
        }
    }

    void process(char * out, const std::vector<char> & src)
    {
        if(!src.empty())
            self().process(out, &src[0], src.size());
    }

    void process(char * out, const std::string & src)
    {
        self().process(out, src.c_str(), src.size());
    }

    inline void process(unsigned char * out, const unsigned char * begin, const unsigned char * end)
    {
        self().process(mstd::pointer_cast<char*>(out), mstd::pointer_cast<const char*>(begin), end - begin);
    }

    inline void process(unsigned char * out, const unsigned char * begin, size_t len)
    {
        self().process(mstd::pointer_cast<char*>(out), mstd::pointer_cast<const char*>(begin), len);
    }

    inline void process(char * out, const char * begin, const char * end)
    {
        self().process(out, begin, end - begin);
    }
private:
    Derived & self() { return *static_cast<Derived*>(this); }
};

#define MCRYPT_CIPHER_CONSTRUCTORS(engine) \
    explicit engine(const std::string & password, const char * ivec = 0) { init(password.c_str(), password.size(), ivec); } \
    explicit engine(const char * password, size_t len, const char * ivec = 0) { init(password, len, ivec); } \
    explicit engine(const unsigned char * password, size_t len, const unsigned char * ivec = 0) { init(mstd::pointer_cast<const char*>(password), len, mstd::pointer_cast<const char*>(ivec)); } \
    explicit engine(const std::vector<char> & password, const char * ivec = 0) { init(&password[0], password.size(), ivec); } \
    explicit engine(const std::vector<unsigned char> & password, const unsigned char * ivec = 0) { init(mstd::pointer_cast<const char*>(&password[0]), password.size(), mstd::pointer_cast<const char*>(ivec)); } \
    /**/

#define MCRYPT_CIPHER_IMPL(engine, base) \
    class MCRYPT_DECL engine : public CipherEngine<engine>, private base { \
    public: \
        MCRYPT_CIPHER_CONSTRUCTORS(engine); \
        using CipherEngine<engine>::process; \
        void process(char * out, const char * begin, size_t len); \
    }; \
    /**/

#define MCRYPT_CIPHER(engine, base) \
    MCRYPT_CIPHER_IMPL(BOOST_PP_CAT(engine, Encrypt), base); \
    MCRYPT_CIPHER_IMPL(BOOST_PP_CAT(engine, Decrypt), base); \
    /**/

}
