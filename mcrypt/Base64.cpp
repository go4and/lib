#include "pch.h"

#include "Base64.h"

namespace mcrypt {

std::string base64(const char * buf, size_t len)
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

std::wstring wbase64(const char * src, size_t len)
{
    return mstd::widen(base64(src, len));
}

std::vector<char> debase64(const std::string & str)
{
    std::vector<char> result(str.length());

    BIO * b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO * bmem = BIO_new_mem_buf(const_cast<char*>(str.c_str()), static_cast<int>(str.length()));
    bmem = BIO_push(b64, bmem);

    int size = BIO_read(bmem, &result[0], static_cast<int>(result.size()));
    result.resize(size);

    BIO_free_all(bmem);

    return result;
}

std::vector<char> debase64(const std::wstring & src)
{
    return debase64(mstd::convert(src));
}

}
