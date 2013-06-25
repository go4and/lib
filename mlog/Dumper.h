/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#if !MLOG_NO_LOGGING
#ifndef MLOG_BUILDING
#include <string.h>

#include <iosfwd>

#include <mstd/itoa.hpp>
#include <mstd/utf8.hpp>
#include <mstd/strings.hpp>
#endif

#include "Config.h"

namespace mlog {

template<class Iterator>
class Dumper {
public:
    template<class Container>
    explicit Dumper(const Container & c)
        : begin_(c.begin()), end_(c.end()) {}
        
    explicit Dumper(const Iterator & begin, const Iterator & end)
        : begin_(begin), end_(end) {}

    template<class Ch, class Tr>
    void operator()(std::basic_ostream<Ch, Tr> & out) const
    {
        const char * ht = mstd::hex_table;
        Iterator i = begin_;
        if(i != end_)
        {
            unsigned char c = static_cast<unsigned char>(*i);
            out << static_cast<Ch>(ht[c >> 4]) << static_cast<Ch>(ht[c & 0xf]);
            for(++i; i != end_; ++i)
            {
                unsigned char c = static_cast<unsigned char>(*i);
                out << static_cast<Ch>(' ') << static_cast<Ch>(ht[c >> 4]) << static_cast<Ch>(ht[c & 0xf]);
            }
        }
    }
private:
    Iterator begin_;
    Iterator end_;
};

template<class Container>
Dumper<typename Container::const_iterator> dump(const Container & c)
{
    return Dumper<typename Container::const_iterator>(c);
}

template<class Iterator>
Dumper<Iterator> dump(Iterator begin, Iterator end)
{
    return Dumper<Iterator>(begin, end);
}

template<class Iterator>
Dumper<Iterator> dump(Iterator begin, size_t len)
{
    return Dumper<Iterator>(begin, begin + len);
}

template<class Ch, class Tr, class Container>
std::basic_ostream<Ch, Tr> & operator<<(std::basic_ostream<Ch, Tr> & out, const Dumper<Container> & dumper)
{
    dumper(out);
    return out;
}

template<class Elem>
struct StringTraits;

template<>
struct StringTraits<char> {
    static size_t length(const char * src)
    {
	return strlen(src);
    }

    static void out(std::ostream & out, const char * src, size_t len)
    {
        out << std::string(src, len);
    }
};

template<>
struct StringTraits<wchar_t> {
    static size_t length(const wchar_t * src)
    {
	return wcslen(src);
    }

    static void out(std::ostream & out, const wchar_t * src, size_t len)
    {
        out << mstd::utf8(std::wstring(src, len));
    }
};

template<class Elem>
class StringDumper {
public:
    explicit StringDumper(const Elem * value)
	: value_(value), len_(StringTraits<Elem>::length(value_)) {}

    explicit StringDumper(const Elem * value, int len)
	: value_(value), len_(len) {}

    void operator()(std::ostream & out) const
    {
        StringTraits<Elem>::out(out, value_, len_);
        out << '[' << dump(value_, value_ + len_) << ']';
    }
private:
    const Elem * value_;
    int len_;
};

template<class Elem>
StringDumper<Elem> dumpStr(const Elem * value)
{
    return StringDumper<Elem>(value);
}

template<class Elem>
StringDumper<Elem> dumpStr(const Elem * value, int len)
{
    return StringDumper<Elem>(value, len);
}

template<class Elem>
std::ostream & operator<<(std::ostream & out, const StringDumper<Elem> & dumper)
{
    dumper(out);
    return out;
}

}
#endif
