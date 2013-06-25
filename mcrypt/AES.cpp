/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include "pch.h"

#include "AES.h"

namespace mcrypt {

const void * Aes128CbcTag::evp()
{
    return EVP_aes_128_cbc();
}

const void * Aes128OfbTag::evp()
{
    return EVP_aes_128_ofb();
}

const void * Aes128CfbTag::evp()
{
    return EVP_aes_128_cfb128();
}

const void * Aes256CbcTag::evp()
{
    return EVP_aes_256_cbc();
}

const void * Aes256OfbTag::evp()
{
    return EVP_aes_256_ofb();
}

const void * Aes256CfbTag::evp()
{
    return EVP_aes_256_cfb128();
}

#if 0
void Blowfish::decryptChunk(boost::uint32_t * data) const
{
    BF_decrypt(reinterpret_cast<BF_LONG*>(data), &key_->value_);
}

void Blowfish::encryptChunk(boost::uint32_t * data) const
{
    BF_encrypt(reinterpret_cast<BF_LONG*>(data), &key_->value_);
}
#endif

}
