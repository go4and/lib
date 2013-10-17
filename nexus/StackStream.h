/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#ifndef NEXUS_BUILDING

#include <boost/type_traits/is_integral.hpp>

#include <mstd/exception.hpp>
#include <mstd/itoa.hpp>

#endif

#include "Buffer.h"

namespace nexus {

class StackStream;
typedef mstd::own_exception<StackStream> StackStreamException;
class PositionTag;
typedef boost::error_info<PositionTag, size_t> ErrorPosition;
class SizeTag;
typedef boost::error_info<SizeTag, size_t> ErrorSize;
class RequiredTag;
typedef boost::error_info<RequiredTag, size_t> ErrorRequired;

class NEXUS_DECL StackStream {
public:
    explicit StackStream(char * begin, size_t size);

    void revert()
    {
        pos_ = begin_;
    }

    template<class T>
    typename boost::enable_if<boost::is_integral<T>, void>::type
    write(T t)
    {
        checkOverflow(sizeof(T));
        *mstd::pointer_cast<T*>(pos_) = t;
        pos_ += sizeof(T);
    }

    void write(const std::vector<char> & buf)
    {
        write(&buf[0], buf.size());
    }

    void write(const std::vector<unsigned char> & buf)
    {
        write(mstd::pointer_cast<const char*>(&buf[0]), buf.size());
    }

    template<size_t N>
    void write(const boost::array<char, N> & buf)
    {
        write(&buf[0], buf.size());
    }

    template<size_t N>
    void write(const boost::array<unsigned char, N> & buf)
    {
        write(mstd::pointer_cast<const char*>(&buf[0]), buf.size());
    }

    void writeShortString(const std::string & str);
    void writeCString(const std::string & str);
    void writeUTF8CString(const std::wstring & str);
    void write(const char * str, size_t len);

    void put(const char * str);
    void put(const std::string & str);
    void put(char ch);

    template<class T>
    typename boost::enable_if<boost::is_integral<T>, void>::type
    put(T t)
    {
        checkOverflow(0x20);
        pos_ += strlen(mstd::itoa(t, pos_));
    }

    template<size_t sz>
    void put(const boost::array<char, sz> & src)
    {
        checkOverflow(sz);
        memcpy(pos_, src.data(), sz);
        pos_ += sz;
    }

    void move(ptrdiff_t offset)
    {
        checkOverflow(offset);
        pos_ += offset;
    }

    char * begin() const { return begin_; }
    char * pos() const { return pos_; }
    char * end() const { return end_; }
    Buffer buffer() const;
private:
    void checkOverflow(size_t required);

    char * begin_;
    char * pos_;
    char * end_;
};

#define NEXUS_STACK_STREAM(name, size) nexus::StackStream name(static_cast<char*>(alloca(size)), (size));

}
