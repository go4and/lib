/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
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
