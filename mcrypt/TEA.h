#pragma once

#ifndef MCRYPT_BUILDING
#include <vector>

#include <mstd/cstdint.hpp>
#endif

#include "Config.h"

namespace mcrypt {

class MCRYPT_DECL TEA {
public:
    TEA(const boost::uint32_t key[4]);
    
    void decryptChunk(boost::uint32_t * data) const;
    void encryptChunk(boost::uint32_t * data) const;
    
    std::vector<unsigned char> & decrypt(std::vector<unsigned char> & buf) const;
    std::vector<unsigned char> & encrypt(std::vector<unsigned char> & buf) const;
    
    void xencryptChunk(boost::uint32_t * data) const;
    void xdecryptChunk(boost::uint32_t * data) const;
    
    std::vector<unsigned char> & xdecrypt(std::vector<unsigned char> & buf) const;
    std::vector<unsigned char> & xencrypt(std::vector<unsigned char> & buf) const;
private:
    boost::uint32_t key_[4];
};

}
