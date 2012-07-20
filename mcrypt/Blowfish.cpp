#include "pch.h"

#include "Blowfish.h"

using namespace std;

namespace mcrypt {

class Blowfish::Key {
public:
    BF_KEY value_;
};

Blowfish::Blowfish(const std::string & password)
{
    init(mstd::pointer_cast<const unsigned char*>(password.c_str()), password.size());
}

Blowfish::Blowfish(const std::vector<unsigned char> & password)
{
    init(&password[0], password.size());
}

Blowfish::Blowfish(const std::vector<char> & password)
{
    init(mstd::pointer_cast<const unsigned char*>(&password[0]), password.size());
}

Blowfish::Blowfish(const char * password, size_t len)
{
    init(mstd::pointer_cast<const unsigned char*>(password), len);
}

Blowfish::Blowfish(unsigned char * password, size_t len)
{
    init(password, len);
}

void Blowfish::init(const unsigned char * password, size_t len)
{
    key_.reset(new Key());
    num_ = 0;
    BF_set_key(&key_->value_, static_cast<int>(len), mstd::pointer_cast<const unsigned char*>(password));
    memset(ivec_, 0, 8);
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

}
