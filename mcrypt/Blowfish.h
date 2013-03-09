#pragma once

#ifndef MCRYPT_BUILDING
#include <string>
#include <vector>

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

#include <mstd/cstdint.hpp>
#endif

#include "Config.h"

namespace mcrypt {

class MCRYPT_DECL Blowfish : private boost::noncopyable {
public:
    explicit Blowfish(const std::string & password, const char * ivec = 0);
    Blowfish(const char * password, size_t len, const char * ivec = 0);
    Blowfish(const unsigned char * password, size_t len, const unsigned char * ivec = 0);
    explicit Blowfish(const std::vector<unsigned char> & password, const unsigned char * ivec = 0);
    explicit Blowfish(const std::vector<char> & password, const char * ivec = 0);

    ~Blowfish();
    
    void encryptChunk(uint32_t * data) const;
    void decryptChunk(uint32_t * data) const;

    void encrypt(const char * begin, const char * end, void * out);
    void decrypt(const char * begin, const char * end, void * out);

    void restart();

    std::vector<char> encryptCFB(const std::vector<char> & src);
    std::vector<char> decryptCFB(const std::vector<char> & src);

    void encryptCFB(const char * begin, const char * end, char * out);
    void decryptCFB(const char * begin, const char * end, char * out);

    std::vector<unsigned char> encryptOFB(const std::vector<unsigned char> & src);
    std::vector<unsigned char> decryptOFB(const std::vector<unsigned char> & src);

    inline void encryptOFB(const char * begin, const char * end, char * out) { encryptOFB(begin, end - begin, out); }
    inline void decryptOFB(const char * begin, const char * end, char * out) { decryptOFB(begin, end - begin, out); }

    void encryptOFB(const char * begin, size_t len, char * out);
    void decryptOFB(const char * begin, size_t len, char * out);
private:
    void init(const unsigned char * password, size_t len, const unsigned char * ivec);

    class Key;
    
    boost::scoped_ptr<Key> key_;
    int num_;
    unsigned char ivec_[8];
};

}
