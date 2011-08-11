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
    Blowfish(const std::string & password);
    ~Blowfish();
    
    void encryptChunk(boost::uint32_t * data) const;
    void decryptChunk(boost::uint32_t * data) const;

    void encrypt(const char * begin, const char * end, char * out) const;
    void decrypt(const char * begin, const char * end, char * out) const;

    void restart();

    std::vector<char> encryptCFB(const std::vector<char> & src);
    std::vector<char> decryptCFB(const std::vector<char> & src);

    std::vector<unsigned char> encryptOFB(const std::vector<unsigned char> & src);
    std::vector<unsigned char> decryptOFB(const std::vector<unsigned char> & src);
private:
    class Key;
    
    boost::scoped_ptr<Key> key_;
    int num_;
};

}
