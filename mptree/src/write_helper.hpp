#pragma once

namespace mptree {

void write_ident(std::streambuf * buf, size_t ident);
void append(char *& out, const char * value, size_t len);

template<class Escape>
void write_escaped(std::streambuf * buf, const char * p, size_t len, const Escape & escape)
{
    char buffer[0x40];
    char * o = buffer;
    char * limit = buffer + sizeof(buffer) - 6;
    for(const char * end = p + len; p != end; ++p)
    {
        char ch = *p;
        if(!escape(o, ch))
            *o++ = ch;
        if(o >= limit)
        {
            buf->sputn(buffer, o - buffer);
            o = buffer;
        }
    }
    if(o != buffer)
        buf->sputn(buffer, o - buffer);
}

template<class Escape>
void write_escaped(std::streambuf * buf, const wchar_t * p, size_t len, const Escape & escape)
{
    char buffer[0x40];
    char * out = buffer;
    char * limit = buffer + sizeof(buffer) - 6;
    const wchar_t ones = static_cast<wchar_t>(-1);
    for(const wchar_t * end = p + len; p != end; ++p)
    {
        wchar_t ch = *p;
        if(!escape(out, ch))
        {
            if (!(ch & (ones ^ 0x7f))) {
                *out = static_cast<char>(ch);
                ++out;
            } else if (ch & (ones ^ 0x07ff)) {
                *out = static_cast<char>(0xe0 | ((ch >> 12) & 0x0f));
                *++out = static_cast<char>(0x80 | ((ch >> 6) & 0x3f));
                *++out = static_cast<char>(0x80 | (ch & 0x3f));
                ++out;
            } else {
                *out = static_cast<char>(0xc0 | ((ch >> 6) & 0x1f));
                *++out = static_cast<char>(0x80 | (ch & 0x3f));
                ++out;
            }
        }
        if(out >= limit)
        {
            buf->sputn(buffer, out - buffer);
            out = buffer;
        }
    }
    if(out != buffer)
        buf->sputn(buffer, out - buffer);
}

}
