#include "pch.hpp"

#include "write_helper.hpp"

namespace mptree {

void write_ident(std::streambuf * buf, size_t ident)
{
    const char * spaces = "                ";
    const size_t spaces_len = 0x10;
    BOOST_ASSERT(strlen(spaces) == spaces_len);
    ident *= 2;
    while(ident > spaces_len)
    {
        buf->sputn(spaces, spaces_len);
        ident -= spaces_len;
    }
    buf->sputn(spaces, ident);
}

void append(char *& out, const char * value, size_t len)
{
    memcpy(out, value, len);
    out += len;
}

}
