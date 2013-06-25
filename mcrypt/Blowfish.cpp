/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
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

namespace {

template<size_t ourSize, size_t realSize>
struct EqSize {
    BOOST_STATIC_ASSERT(ourSize == realSize);
};

}

BlowfishChunk::BlowfishChunk(const char * key, size_t keySize)
{
    BOOST_STATIC_ASSERT((sizeof(EqSize<Context::size, sizeof(BF_KEY)>)));
    BF_KEY * bfkey = static_cast<BF_KEY*>(context_.address());
    BF_set_key(bfkey, keySize, mstd::pointer_cast<const unsigned char*>(key));
}

void BlowfishChunk::decryptChunk(boost::uint32_t * data) const
{
    const BF_KEY * bfkey = static_cast<const BF_KEY*>(context_.address());
    BF_decrypt(reinterpret_cast<BF_LONG*>(data), bfkey);
}

void BlowfishChunk::encryptChunk(boost::uint32_t * data) const
{
    const BF_KEY * bfkey = static_cast<const BF_KEY*>(context_.address());
    BF_encrypt(reinterpret_cast<BF_LONG*>(data), bfkey);
}

}
