/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#include <boost/config.hpp>

#include <string.h>
#include <wchar.h>

#include <algorithm>

#ifndef BOOST_NO_EXCEPTIONS
#include <typeinfo>
#endif

#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_signed.hpp>

#include "config.hpp"

namespace mstd {

    extern MSTD_DECL const char * const hex_table;

    namespace detail {
        const size_t blockLen = 4;
        const size_t blockPow = 10000;

        extern MSTD_DECL const char * const digitBlocks;

        template<class T, class Char>
        Char * pitoa(T i, Char * buf)
        {
            Char * p = buf;
            while(i) {
                const char * src = digitBlocks + (i % blockPow) * blockLen;
                std::copy(src, src + blockLen, p);
                p += blockLen;
                i = static_cast<T>(i / blockPow);
            }
            if(*(p - 1) == '0')
            {
                --p;
                if(*(p - 1) == '0')
                {
                    --p;
                    if(*(p - 1) == '0')
                        --p;
                }
            }
            std::reverse(buf, p);
            return p;
        }
    }

template<class T, class Char>
typename boost::disable_if<boost::is_signed<T>, Char*>::type
itoa(T i, Char * buf)
{
    Char * p = buf;
    if(i)
        p = detail::pitoa(i, p);
    else
        *p++ = '0';

    *p = 0;
    return buf;
}

template<class T, class Char>
typename boost::disable_if<boost::is_signed<T>, Char*>::type
itoa16(T i, Char * buf)
{
    Char * p = buf;
    if(i) {
        while(i) {
            *p++ = hex_table[i & 0xf];
            i >>= 4;
        }
        std::reverse(buf, p);
    } else
        *p++ = '0';

    *p = 0;
    return buf;
}

template<class T, class Char>
typename boost::enable_if<boost::is_signed<T>, Char*>::type
itoa(T i, Char * buf)
{
    Char * p = buf;
    if(i)
    {
        if(i < 0)
        {
            i = -i;
            *p = '-';
            ++p;
        }
        p = detail::pitoa(i, p);
    } else {
        *p++ = '0';
    }

    *p = 0;
    return buf;
}

#ifndef BOOST_NO_EXCEPTIONS
class bad_str2int_cast : public std::bad_cast {
public:
    bad_str2int_cast() {}

    virtual const char *what() const throw()
    {
        return "bad str2int cast";
    }

    virtual ~bad_str2int_cast() throw() {}
};
#endif

template<class T, class Check, class It>
T str2int10_impl(It inp, It end)
{
    typedef typename std::iterator_traits<It>::reference ref;
    T result = 0;
    for(; inp != end; ++inp)
    {
        ref c = *inp;
#ifndef BOOST_NO_EXCEPTIONS
        if(Check::value && (c < '0' || c > '9'))
            throw bad_str2int_cast();
#endif
        result = result * 10 + (c - '0');
    }
    return result;
}

template<class T, class Check, class It>
typename boost::enable_if<boost::is_signed<T>, T>::type
str2int10_i1(It inp, It end)
{
    if(inp != end && *inp == '-')
        return -str2int10_impl<T, Check>(++inp, end);
    else
        return str2int10_impl<T, Check>(inp, end);
}

template<class T, class Check, class It>
typename boost::disable_if<boost::is_signed<T>, T>::type
str2int10_i1(It inp, It end)
{
    return str2int10_impl<T, Check>(inp, end);
}

template<class T, class It>
T str2int10(It inp, It end)
{
    return str2int10_i1<T, boost::mpl::false_>(inp, end);
}

template<class T, class It>
T str2int10_checked(It inp, It end)
{
    return str2int10_i1<T, boost::mpl::true_>(inp, end);
}

template<class T, class Ch>
T str2int10(const Ch * inp, size_t len)
{
    return str2int10_i1<T, boost::mpl::false_>(inp, inp + len);
}

template<class T, class Ch>
T str2int10_checked(const Ch * inp, size_t len)
{
    return str2int10_i1<T, boost::mpl::true_>(inp, inp + len);
}

template<class T, class C>
T str2int10(const C & c)
{
    return str2int10_i1<T, boost::mpl::false_>(c.begin(), c.end());
}

template<class T, class C>
T str2int10_checked(const C & c)
{
    return str2int10_i1<T, boost::mpl::true_>(c.begin(), c.end());
}

template<class T>
T str2int10(const char * c)
{
    return str2int10_i1<T, boost::mpl::false_>(c, c + strlen(c));
}

template<class T>
T str2int10(char * c)
{
    return str2int10_i1<T, boost::mpl::false_>(c, c + strlen(c));
}

template<class T>
T str2int10_checked(const char * c)
{
    return str2int10_i1<T, boost::mpl::true_>(c, c + strlen(c));
}

template<class T>
T str2int10_checked(char * c)
{
    return str2int10_i1<T, boost::mpl::true_>(c, c + strlen(c));
}

template<class T>
T str2int10(const wchar_t * c)
{
    return str2int10_i1<T, boost::mpl::false_>(c, c + wcslen(c));
}

template<class T>
T str2int10_checked(const wchar_t * c)
{
    return str2int10_i1<T, boost::mpl::true_>(c, c + wcslen(c));
}

template<class T>
T str2int10(wchar_t * c)
{
    return str2int10_i1<T, boost::mpl::false_>(c, c + wcslen(c));
}

template<class T>
T str2int10_checked(wchar_t * c)
{
    return str2int10_i1<T, boost::mpl::true_>(c, c + wcslen(c));
}

template<class T, class It, class Value>
typename boost::disable_if<boost::is_signed<T>, T>::type
inline str2int16_impl(It inp, const It & end, Value * value)
{
    T result = 0;
    for(; inp != end; ++inp)
    {
        result = result << 4;
        Value ch = *inp;
        if(ch <= '9')
            result += (ch - '0');
        else if(ch <= 'F')
            result += (ch - 'A' + 10);
        else
            result += (ch - 'a' + 10);
    }
    return result;
}

template<class T, class It, class Value>
typename boost::enable_if<boost::is_signed<T>, T>::type
inline str2int16_impl(It inp, const It & end, Value * value)
{
    T result = 0;
    T sign = 1;
    if(inp != end && *inp == '-')
    {
        sign = -1;
        ++inp;
    }
    for(; inp != end; ++inp)
    {
        result = result << 4;
        Value ch = *inp;
        if(ch <= '9')
            result += (ch - '0');
        else if(ch <= L'F')
            result += (ch - 'A' + 10);
        else
            result += (ch - 'a' + 10);
    }
    return result * sign;
}

template<class T, class It>
inline T str2int16(const It & inp, const It & end)
{
    return str2int16_impl<T>(inp, end, &*inp);
}

template<class T, class It>
inline T str2int16(const It & begin, size_t len)
{
    return str2int16<T>(begin, begin + len);
}

template<class T>
inline T str2int16(const wchar_t * c)
{
    return str2int16<T>(c, wcslen(c));
}

template<class T>
inline T str2int16(const char * c)
{
    return str2int16<T>(c, strlen(c));
}

}
