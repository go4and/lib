#pragma once

namespace mptree {

void write_value(std::streambuf * buf, const std::wstring & value);

inline void write_value(std::streambuf * buf, bool value)
{
    if(value)
        buf->sputn("true", 4);
    else
        buf->sputn("false", 5);
}

template<class T>
typename boost::enable_if<boost::is_integral<T>, void>::type write_value(std::streambuf * buf, const T & value)
{
    char buffer[0x40];
    mstd::itoa(value, buffer);
    buf->sputn(buffer, strlen(buffer));
}

template<class T>
typename boost::enable_if<boost::is_enum<T>, void>::type write_value(std::streambuf * buf, const T & value)
{
    const char * buffer = name(value);
    buf->sputn(buffer, strlen(buffer));
}

template<class T>
void write_value(std::ostream & out, const char * name, const boost::optional<T> & value, size_t ident)
{
    if(value)
        write_value(out, name, *value, ident);
}

}
