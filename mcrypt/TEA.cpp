/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include "pch.h"

#include "TEA.h"

using namespace boost;

namespace mcrypt {

TEA::TEA(const uint32_t key[4])
{
    memcpy(key_, key, sizeof(key_));
}

void TEA::encryptChunk(uint32_t * data) const
{
    uint32_t v0 = data[0], v1 = data[1], sum = 0;
    uint32_t delta = 0x9e3779b9;
    uint32_t k0 = key_[0], k1 = key_[1], k2 = key_[2], k3 = key_[3];
    for(int i = 0; i < 32; ++i)
    {
        sum += delta;
        v0 += ((v1<<4) + k0) ^ (v1 + sum) ^ ((v1>>5) + k1);
        v1 += ((v0<<4) + k2) ^ (v0 + sum) ^ ((v0>>5) + k3);  /* end cycle */
    }
    data[0] = v0; data[1] = v1;
}

void TEA::decryptChunk(uint32_t * data) const
{
    uint32_t v0 = data[0], v1 = data[1], sum=0xC6EF3720;
    uint32_t delta = 0x9e3779b9;                     /* a key schedule constant */
    uint32_t k0 = key_[0], k1 = key_[1], k2 = key_[2], k3 = key_[3];   /* cache key */
    for(int i = 0; i < 32; ++i)
    {
        v1 -= ((v0<<4) + k2) ^ (v0 + sum) ^ ((v0>>5) + k3);
        v0 -= ((v1<<4) + k0) ^ (v1 + sum) ^ ((v1>>5) + k1);
        sum -= delta;                                   /* end cycle */
    }
    data[0] = v0; data[1] = v1;
}

void TEA::xencryptChunk(boost::uint32_t * data) const
{
    boost::uint32_t v0 = data[0], v1 = data[1];
    boost::uint32_t sum = 0, delta = 0x9E3779B9;
    for(int i = 0; i < 32; ++i)
    {
        v0 += (((v1 << 4 ^ v1 >> 5) + v1) ^ sum) + key_[sum & 3];
        sum += delta;
        v1 += (((v0 << 4 ^ v0 >> 5) + v0) ^ sum) + key_[sum>>11 & 3];
    }
    data[0] = v0; 
    data[1] = v1;
}

void TEA::xdecryptChunk(boost::uint32_t * data) const
{
    boost::uint32_t v0 = data[0], v1 = data[1];
    boost::uint32_t delta = 0x9E3779B9, sum = delta * 32;
    for(int i = 0; i < 32; ++i)
    {
        v1 -= (((v0 << 4 ^ v0 >> 5) + v0) ^ sum) + key_[sum >> 11 & 3];
        sum -= delta;
        v0 -= (((v1 << 4 ^ v1 >> 5) + v1) ^ sum) + key_[sum & 3];
    }
    data[0] = v0;
    data[1] = v1;
}

template<class F>
std::vector<unsigned char> & process(std::vector<unsigned char> & src, const TEA * tea, F func)
{
    uint32_t * p = reinterpret_cast<uint32_t*>(&src[0]);
    uint32_t * e = p + src.size() / 8 * 2;
    for(; p < e; p += 2)
        (tea->*func)(p);
    return src;
}

std::vector<unsigned char> & TEA::encrypt(std::vector<unsigned char> & src) const
{
    return process(src, this, &TEA::encryptChunk);
}
    
std::vector<unsigned char> & TEA::decrypt(std::vector<unsigned char> & src) const
{
    return process(src, this, &TEA::decryptChunk);
}

std::vector<unsigned char> & TEA::xencrypt(std::vector<unsigned char> & src) const
{
    return process(src, this, &TEA::xencryptChunk);
}

std::vector<unsigned char> & TEA::xdecrypt(std::vector<unsigned char> & src) const
{
    return process(src, this, &TEA::xdecryptChunk);
}

}
