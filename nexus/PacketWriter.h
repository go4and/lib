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

#include <mstd/null.hpp>
#include <mstd/pointer_cast.hpp>
#include <mstd/utf8.hpp>

namespace nexus {

inline void writeBytes(char *& pos, const void * bytes, size_t len)
{
#ifdef NDEBUG
    memcpy(pos, bytes, len);
#else
    const char * p = static_cast<const char*>(bytes);
    std::copy(p, p + len, pos);
#endif

    pos += len;
}

template<class Ch, class Result>
struct EnableIfByte : public boost::enable_if<boost::mpl::or_<boost::is_same<Ch, char>, boost::is_same<Ch, unsigned char> >, Result> {};

template<class Ch>
inline typename EnableIfByte<Ch, void>::type
writeBytes(std::vector<Ch> & out, const void * bytes, size_t len)
{
    const char * begin = static_cast<const char*>(bytes);
    out.insert(out.end(), begin, begin + len);
}

template<class Ch>
inline typename EnableIfByte<Ch, size_t>::type
prepareBytes(std::vector<Ch> & out, size_t len)
{
    size_t oldSize = out.size();
    out.resize(oldSize + len);
    return oldSize;
}

template<class Ch>
inline typename EnableIfByte<Ch, std::back_insert_iterator<std::vector<Ch>>>::type
writingIterator(std::vector<Ch> & out)
{
    return std::back_inserter(out);
}

template<class Ch, class Len>
inline typename EnableIfByte<Ch, void>::type
commitLen(std::vector<Ch> & out, size_t oldSize, Len*)
{
    Len size = static_cast<Len>(out.size() - oldSize - sizeof(Len));
    memcpy(&out[oldSize], &size, sizeof(Len));
}

template<class T, class U>
inline typename boost::enable_if<boost::mpl::and_<boost::is_pod<T>, boost::mpl::not_<boost::is_pointer<T> > >, void>::type
write(U & u, const T & t)
{
    writeBytes(u, &t, sizeof(T));
}

template<class T, class U>
inline typename boost::enable_if<boost::mpl::and_<boost::is_pod<T>, boost::mpl::not_<boost::is_pointer<T> > >, void>::type
write(U & u, const T * t, size_t size)
{
    writeBytes(u, t, sizeof(T) * size);
}

template<class T, class U>
inline void write(U & u, const T * t, const T * end)
{
    write(pos, t, end - t);
}

template<class U>
inline void writeWCString(U & u, const std::wstring & str)
{
    writeBytes(u, str.c_str(), (str.length() + 1) * 2);
}

template<class U>
inline void writeCString(U & u, const char * str, size_t len)
{
    BOOST_ASSERT(!str[len]);

    writeBytes(u, str, ++len);
}

template<class U>
inline void writeCString(U & u, const char * str)
{
    writeCString(u, str, strlen(str));
}

template<class U>
inline void writeCString(U & u, const std::string & str)
{
    writeCString(u, str.c_str(), str.length());
}

template<class Len, class U>
inline void writeLenString(U & out, const char * str, size_t len)
{
    write<Len>(out, static_cast<Len>(len));
    writeBytes(out, str, len);
}

template<class Len, class U>
inline void writeLenString(U & out, const char * str)
{
    writeLenString<Len>(out, str, strlen(str));
}

template<class Len, class U>
inline void writeLenString(U & out, const std::string & str)
{
    writeLenString<Len>(out, str.c_str(), str.length());
}

template<class Len, class U, class It>
inline void writeLenUTFString(U & out, It begin, It end)
{
    auto mark = prepareBytes(out, sizeof(Len));
    mstd::utf8(begin, end, writingIterator(out));
    commitLen(out, mark, mstd::null<Len>());
}

template<class Len, class U, class It>
inline void writeLenUTFString(U & out, It begin, size_t len)
{
    writeLenUTFString<Len>(out, begin, begin + len);
}

template<class Len, class U>
inline void writeLenUTFString(U & out, const std::wstring & str)
{
    writeLenUTFString<Len>(out, str.begin(), str.end());
}

template<class U>
inline void writePacked(U & p, boost::uint32_t size)
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
