#include "pch.h"

#include "Blowfish.h"

namespace mcrypt {

void BlowfishBase::init(const char * password, size_t len, const char * ivec)
{
    BOOST_STATIC_ASSERT(sizeof(BF_KEY) == Key::size);
    num_ = 0;
    BF_set_key(static_cast<BF_KEY*>(key_.address()), static_cast<int>(len), mstd::pointer_cast<const unsigned char*>(password));
    if(!ivec)
        memset(ivec_, 0, sizeof(ivec_));
    else
        memcpy(ivec_, ivec, sizeof(ivec_));
}

void BlowfishCbcEncrypt::process(char * out, const char * begin, size_t len)
{
    BF_cbc_encrypt(mstd::pointer_cast<const unsigned char *>(begin),
                   mstd::pointer_cast<unsigned char *>(out),
                   static_cast<int>(len),
                   static_cast<BF_KEY*>(key_.address()), ivec_, BF_ENCRYPT);
}

void BlowfishCbcDecrypt::process(char * out, const char * begin, size_t len)
{
    BF_cbc_encrypt(mstd::pointer_cast<const unsigned char *>(begin),
                   mstd::pointer_cast<unsigned char *>(out),
                   static_cast<int>(len),
                   static_cast<BF_KEY*>(key_.address()), ivec_, BF_DECRYPT);
}

void BlowfishOfbEncrypt::process(char * out, const char * begin, size_t len)
{
    BF_ofb64_encrypt(mstd::pointer_cast<const unsigned char *>(begin),
                     mstd::pointer_cast<unsigned char *>(out),
                     static_cast<int>(len),
                     static_cast<BF_KEY*>(key_.address()), ivec_, &num_);
}

void BlowfishOfbDecrypt::process(char * out, const char * begin, size_t len)
{
    BF_ofb64_encrypt(mstd::pointer_cast<const unsigned char *>(begin),
                     mstd::pointer_cast<unsigned char *>(out),
                     static_cast<int>(len),
                     static_cast<BF_KEY*>(key_.address()), ivec_, &num_);
}

void BlowfishCfbEncrypt::process(char * out, const char * begin, size_t len)
{
    BF_cfb64_encrypt(mstd::pointer_cast<const unsigned char *>(begin),
                     mstd::pointer_cast<unsigned char *>(out),
                     static_cast<int>(len),
                     static_cast<BF_KEY*>(key_.address()), ivec_, &num_, BF_ENCRYPT);
}

void BlowfishCfbDecrypt::process(char * out, const char * begin, size_t len)
{
    BF_cfb64_encrypt(mstd::pointer_cast<const unsigned char *>(begin),
                     mstd::pointer_cast<unsigned char *>(out),
                     static_cast<int>(len),
                     static_cast<BF_KEY*>(key_.address()), ivec_, &num_, BF_DECRYPT);
}

#if 0
class Blowfish::Key {
public:
    BF_KEY value_;
};

Blowfish::Blowfish(const std::string & password, const char * ivec)
{
    init(mstd::pointer_cast<const unsigned char*>(password.c_str()), password.size(), mstd::pointer_cast<const unsigned char*>(ivec));
}

Blowfish::Blowfish(const std::vector<unsigned char> & password, const unsigned char * ivec)
{
    init(&password[0], password.size(), ivec);
}

Blowfish::Blowfish(const std::vector<char> & password, const char * ivec)
{
    init(mstd::pointer_cast<const unsigned char*>(&password[0]), password.size(), mstd::pointer_cast<const unsigned char*>(ivec));
}

Blowfish::Blowfish(const char * password, size_t len, const char * ivec)
{
    init(mstd::pointer_cast<const unsigned char*>(password), len, mstd::pointer_cast<const unsigned char*>(ivec));
}

Blowfish::Blowfish(const unsigned char * password, size_t len, const unsigned char * ivec)
{
    init(password, len, ivec);
}

void Blowfish::init(const unsigned char * password, size_t len, const unsigned char * ivec)
{
    key_.reset(new Key());
    num_ = 0;
    BF_set_key(&key_->value_, static_cast<int>(len), mstd::pointer_cast<const unsigned char*>(password));
    if(!ivec)
        memset(ivec_, 0, sizeof(ivec_));
    else
        memcpy(ivec_, ivec, sizeof(ivec_));
}

Blowfish::~Blowfish() {}

void Blowfish::decryptChunk(boost::uint32_t * data) const
{
    BF_decrypt(reinterpret_cast<BF_LONG*>(data), &key_->value_);
}

void Blowfish::encryptChunk(boost::uint32_t * data) const
{
    BF_encrypt(reinterpret_cast<BF_LONG*>(data), &key_->value_);
}

void process(const char * begin, const char * end, void * out, BF_KEY * key, unsigned char * ivec, int enc)
{
    BF_cbc_encrypt(mstd::pointer_cast<const unsigned char *>(begin),
                   static_cast<unsigned char *>(out),
                   end - begin, key, ivec, enc);
}

void Blowfish::encrypt(const char * begin, const char * end, void * out)
{
    return process(begin, end, out, &key_->value_, ivec_, BF_ENCRYPT);
}

void Blowfish::decrypt(const char * begin, const char * end, void * out)
{
    return process(begin, end, out, &key_->value_, ivec_, BF_DECRYPT);
}

void Blowfish::restart()
{
    num_ = 0;
}

std::vector<char> processCFB(const std::vector<char> & src, BF_KEY * key, int * num, int enc)
{
    if(src.empty())
        return src;
    unsigned char ivec[8];
    memset(ivec, 0, 8);
    std::vector<char> result(src.size());
    BF_cfb64_encrypt(mstd::pointer_cast<const unsigned char*>(&src[0]),
                     mstd::pointer_cast<unsigned char*>(&result[0]),
                     static_cast<long>(src.size()), key, ivec, num, enc);
    return result;
}

std::vector<char> Blowfish::encryptCFB(const std::vector<char> &src)
{
    return processCFB(src, &key_->value_, &num_, BF_ENCRYPT);
}

std::vector<char> Blowfish::decryptCFB(const std::vector<char> &src)
{
    return processCFB(src, &key_->value_, &num_, BF_DECRYPT);
}

void Blowfish::encryptCFB(const char * begin, const char * end, char * out)
{
    BF_cfb64_encrypt(mstd::pointer_cast<const unsigned char*>(begin),
                     mstd::pointer_cast<unsigned char*>(out),
                     static_cast<long>(end - begin),
                     &key_->value_, ivec_, &num_, BF_ENCRYPT);
}

void Blowfish::decryptCFB(const char * begin, const char * end, char * out)
{
    BF_cfb64_encrypt(mstd::pointer_cast<const unsigned char*>(begin),
                     mstd::pointer_cast<unsigned char*>(out),
                     static_cast<long>(end - begin),
                     &key_->value_, ivec_, &num_, BF_DECRYPT);
}

std::vector<unsigned char> Blowfish::decryptOFB(const std::vector<unsigned char> &src)
{
    return encryptOFB(src);
}

std::vector<unsigned char> Blowfish::encryptOFB(const std::vector<unsigned char> & src)
{
    if(src.empty())
        return src;
    unsigned char ivec[8];
    memset(ivec, 0, 8);
    std::vector<unsigned char> result(src.size());
    BF_ofb64_encrypt(&src[0], &result[0], static_cast<long>(src.size()), &key_->value_, ivec, &num_);
    return result;
}

void Blowfish::encryptOFB(const char * begin, size_t len, char * out)
{
    BF_ofb64_encrypt(mstd::pointer_cast<const unsigned char*>(begin),
                     mstd::pointer_cast<unsigned char*>(out),
                     static_cast<long>(len),
                     &key_->value_, ivec_, &num_);
}

void Blowfish::decryptOFB(const char * begin, size_t len, char * out)
{
    BF_ofb64_encrypt(mstd::pointer_cast<const unsigned char*>(begin),
                     mstd::pointer_cast<unsigned char*>(out),
                     static_cast<long>(len),
                     &key_->value_, ivec_, &num_);
}
#endif

}
