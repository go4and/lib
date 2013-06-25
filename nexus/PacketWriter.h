/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#include <boost/static_assert.hpp>
#include <boost/type_traits/is_pod.hpp>
#include <boost/type_traits/is_pointer.hpp>

#include <mstd/pointer_cast.hpp>

namespace nexus {

template<class T>
void write(char *& pos, const T & t)
{
    BOOST_STATIC_ASSERT(boost::is_pod<T>::value && !boost::is_pointer<T>::value);

#ifdef NDEBUG
    memcpy(pos, &t, sizeof(t));
#else
    const char * p = mstd::pointer_cast<const char*>(&t);
    std::copy(p, p + sizeof(t), pos);
#endif

    pos += sizeof(t);
}

inline void writeWCString(char *& out, const std::wstring & str)
{
    size_t len = (str.length() + 1) * 2;
    memcpy(out, str.c_str(), len);
    out += len;
}

inline void writeCString(char *& out, const char * str, size_t len)
{
    BOOST_ASSERT(!str[len]);

    ++len;
    memcpy(out, str, len);
    out += len;
}

inline void writeCString(char *& out, const char * str)
{
    writeCString(out, str, strlen(str));
}

inline void writeCString(char *& out, const std::string & str)
{
    writeCString(out, str.c_str(), str.length());
}

inline void writeLenString(char *& out, const char * str, size_t len)
{
    write<uint16_t>(out, len);
    memcpy(out, str, len);
    out += len;
}

inline void writeLenString(char *& out, const char * str)
{
    writeLenString(out, str, strlen(str));
}

inline void writeLenString(char *& out, const std::string & str)
{
    writeLenString(out, str.c_str(), str.length());
}

inline void writePacked(char *& p, boost::uint32_t size)
{
    if(size <= 0x7fff)
        write(p, static_cast<boost::uint16_t>(size));
    else {
        write(p, static_cast<boost::uint16_t>(size | 0x8000));
        write(p, static_cast<boost::uint16_t>(size >> 15));
    }
}

size_t compressSize(size_t len);
size_t compressSize(const char * begin, const char * end);

size_t compress(const void * data, size_t len, void * out, size_t outSize);
inline size_t compress(const char * begin, const char * end, char * out, size_t outSize) { return compress(begin, end - begin, out, outSize); }
std::string compress(const std::string & input);

void compress(const void * data, size_t len, std::vector<char> & out);
inline void compress(const std::vector<char> & input, std::vector<char> & out) { compress(&input[0], input.size(), out); }

}
