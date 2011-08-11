#pragma once

#ifndef MCRYPT_BUILDING
#include <vector> 
#endif

#include "Config.h"

namespace mcrypt {

class MCRYPT_DECL Crypt {
public:
    Crypt(const std::vector<unsigned char> & key);

    void decrypt(unsigned char * begin, unsigned char * end);
    void encrypt(unsigned char * begin, unsigned char * end);
private:
    std::vector<unsigned char> key_;    
};

}
