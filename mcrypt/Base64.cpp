#include "pch.h"

#include "Base64.h"

namespace mcrypt {

namespace {

const char * base64chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char * base64urlChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

}

std::string oldbase64(const void * buf, size_t len)
{
    BIO *bmem, *b64;
    BUF_MEM *bptr;

    b64 = BIO_new(BIO_f_base64());
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(b64, buf, static_cast<int>(len));
    BOOST_VERIFY(BIO_flush(b64) > 0);
    BIO_get_mem_ptr(b64, &bptr);

    std::string result(bptr->data, bptr->data + bptr->length);

    BIO_free_all(b64);

    return result;
}

std::string base64(const void * rbuf, size_t len, bool url)
{
    const char * chars = url ? base64urlChars : base64chars;
    size_t completeBlocks = len / 3;
    size_t remainder = len - completeBlocks * 3;
    size_t outLen = (completeBlocks + (remainder != 0)) * 4;
    std::string result(outLen, 4);
    uint32_t v = 0;
    size_t w = 0;
    const char * buf = static_cast<const char*>(rbuf);
    for(size_t i = 0; i != completeBlocks; ++i, buf += 3)
    {
        memcpy(&v, buf, 3);
        result[w++] = chars[(v >> 2) & 0x3f];
        result[w++] = chars[((v & 0x03) << 4) | ((v >> 12) & 0x0f)];
        result[w++] = chars[((v & 0xf00) >> 6) | ((v >> 22) & 0x03)];
        result[w++] = chars[(v >> 16) & 0x3f];
    }
    if(remainder == 1)
    {
        memcpy(&v, buf, 1);
        v &= 0xff;
        result[w++] = chars[(v >> 2) & 0x3f];
        result[w++] = chars[((v & 0x03) << 4) | ((v >> 12) & 0x0f)];
        result[w++] = '=';
        result[w++] = '=';
    } else if(remainder == 2)
    {
        memcpy(&v, buf, 2);
        v &= 0xffff;
        result[w++] = chars[(v >> 2) & 0x3f];
        result[w++] = chars[((v & 0x03) << 4) | ((v >> 12) & 0x0f)];
        result[w++] = chars[((v & 0xf00) >> 6) | ((v >> 22) & 0x03)];
        result[w++] = '=';
    }
    BOOST_ASSERT(result == oldbase64(rbuf, len));
    return result;
}

std::wstring wbase64(const void * src, size_t len, bool url)
{
    return mstd::widen(base64(src, len, url));
}

unsigned char debase64char(char ch, bool allowEq)
{
    if(ch >= 'A' && ch <= 'Z')
        return ch - 'A';
    if(ch >= 'a' && ch <= 'z')
        return ch - 'a' + 26;
    if(ch >= '0' && ch <= '9')
        return ch - '0' + 52;
    if(ch == '+' || ch == '-')
        return 62;
    if(ch == '/' || ch == '_')
        return 63;
    if(allowEq && ch == '=')
        return 0;
    BOOST_ASSERT(false);
    return 0;
}

inline void debase64block(const char *& input, unsigned char *& out, bool allowEq)
{
    uint32_t c1 = debase64char(*input++, allowEq);
    uint32_t c2 = debase64char(*input++, allowEq);
    uint32_t c3 = debase64char(*input++, allowEq);
    uint32_t c4 = debase64char(*input++, allowEq);
    *out++ = (c1 << 2) | (c2 >> 4);
    *out++ = ((c2 & 0x0f) << 4) | (c3 >> 2);
    *out++ = ((c3 & 0x03) << 6) | (c4);
}

size_t debase64(const char * str, size_t len, void * rout)
{
    BOOST_ASSERT((len & 3) == 0);
    unsigned char * out = static_cast<unsigned char*>(rout);
    size_t blocks = len / 4;
    if(!blocks)
        return 0;
    const char * input = str;
    while(blocks-- > 1)
        debase64block(input, out, false);

    unsigned char temp[0x03];
    unsigned char * tout = temp;
    debase64block(input, tout, true);

    if(input[-1] == '=')
    {
        if(input[-2] == '=')
            *out++ = *temp;
        else {
            memcpy(out, temp, 2);
            out += 2;
        }
    } else {
        memcpy(out, temp, 3);
        out += 3;
    }

    size_t result = out - static_cast<unsigned char*>(rout);

    BOOST_ASSERT(base64(rout, result, false) == std::string(str, len) || base64(rout, result, true) == std::string(str, len));

    return result;
}

}
