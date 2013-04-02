#include "pch.h"

#include "Blowfish.h"

namespace mcrypt {

const void * BlowfishCbcTag::evp()
{
    return EVP_bf_cbc();
}

const void * BlowfishOfbTag::evp()
{
    return EVP_bf_ofb();
}

const void * BlowfishCfbTag::evp()
{
    return EVP_bf_cfb64();
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
