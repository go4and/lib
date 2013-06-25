/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

#include "Cipher.h"

namespace mcrypt {

MCRYPT_CIPHER(Aes128Cbc);
MCRYPT_CIPHER(Aes128Ofb);
MCRYPT_CIPHER(Aes128Cfb);
MCRYPT_CIPHER(Aes256Cbc);
MCRYPT_CIPHER(Aes256Ofb);
MCRYPT_CIPHER(Aes256Cfb);

}
