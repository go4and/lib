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

#include <string.h>

#include <algorithm>
#include <exception>
#include <string>
#include <vector>

#include <boost/config.hpp>

#include <boost/array.hpp>
#include <boost/assert.hpp>
#include <boost/static_assert.hpp>

#include <boost/mpl/bool.hpp>
#include <boost/mpl/or.hpp>

#include <boost/type_traits/is_pod.hpp>
#include <boost/type_traits/is_same.hpp>

#include <boost/utility/enable_if.hpp>

#include <mstd/cstdint.hpp>
#include <mstd/hton.hpp>
#include <mstd/pointer_cast.hpp>
#include <mstd/strings.hpp>

#endif

#include "Config.h"

namespace nexus {

namespace detail {

    template<class T>
    struct IsVector : public boost::mpl::false_ {
    };
    
    template<class T>
    struct IsVector<std::vector<T> > : public boost::mpl::true_ {
    };

}

class NEXUS_DECL InflateException : public std::exception {
public:
    const char * what() throw() {
        return "inflate failed";
    }

    ~InflateException() throw () {}
};

class NEXUS_DECL InvalidStringException : public std::exception {
public:
    const char * what() const throw()
    {
        return "invalid string exception";
    }
    
    ~InvalidStringException() throw () {}
};

class NEXUS_DECL ReaderUnderflowException : public std::exception {
public:
    const char * what() const throw()
    {
        return "reader underflow exception";
    }
    
    ~ReaderUnderflowException() throw () {}
};

class NEXUS_DECL PacketReader {
public:
    explicit PacketReader()
        : pos_(0), end_(0) {}

    explicit PacketReader(const std::vector<char> & inp)
        : pos_(inp.empty() ? 0 : &inp[0]), end_(pos_ + inp.size()) {}

    explicit PacketReader(const std::vector<char> & inp, size_t size)
        : pos_(inp.empty() ? 0 : &inp[0]), end_(pos_ + size) {}

    explicit PacketReader(const std::vector<unsigned char> & inp)
        : pos_(inp.empty() ? 0 : mstd::pointer_cast<const char*>(&inp[0])), end_(pos_ + inp.size()) {}

    explicit PacketReader(const std::vector<unsigned char> & inp, size_t size)
        : pos_(inp.empty() ? 0 : mstd::pointer_cast<const char*>(&inp[0])), end_(pos_ + size) {}

    explicit PacketReader(const char * begin, const char * end)
        : pos_(begin), end_(end) {}
    
    explicit PacketReader(const char * begin, size_t len)
        : pos_(begin), end_(pos_ + len) {}

    size_t left() const
    {
        return end_ - pos_;
    }

    void revert(size_t count)
    {
        pos_ -= count;
    }

    void mark()
    {
        marked_ = pos_;
    }

    void revert()
    {
        pos_ = marked_;
    }
    
    void skip(size_t count)
    {
        pos_ += count;
    }
    
    PacketReader subreader(size_t offset, size_t len)
    {
        return PacketReader(pos_ + offset, pos_ + offset + len);
    }
    
    std::vector<char> vector() const
    {
        return std::vector<char>(pos_, end_);
    }

	uint8_t readUInt8()
    {
		return read<uint8_t>();
    }

	uint16_t readUInt16()
    {
		return read<uint16_t>();
    }

	uint32_t readUInt32()
    {
		return read<uint32_t>();
    }

	uint64_t readUInt64()
    {
		return read<uint64_t>();
    }

	int8_t readInt8()
	{
		return read<int8_t>();
	}

	int16_t readInt16()
	{
		return read<int16_t>();
	}

	int32_t readInt32()
	{
		return read<int32_t>();
	}

	int64_t readInt64()
	{
		return read<int64_t>();
	}

	template<class T>
    typename boost::enable_if<boost::is_pod<T>, T>::type
    read()
    {
        T result;
        memcpy(&result, pos_, sizeof(T));
        pos_ += sizeof(T);
        BOOST_ASSERT(pos_ <= end_);
        return result;
    }
    
    template<class T>
    typename boost::enable_if<boost::is_pod<T>, T>::type
    peek()
    {
        const char * stop = pos_ + sizeof(T);
        (void)stop;
        BOOST_ASSERT(stop <= end_);
        return *mstd::pointer_cast<const T*>(pos_);
    }

    template<class T>
    typename boost::enable_if<boost::is_pod<T>, T>::type
    checkedRead()
    {
        if(pos_ + sizeof(T) > end_)
            throw ReaderUnderflowException();
        T result;
        memcpy(&result, pos_, sizeof(T));
        pos_ += sizeof(T);
        return result;
    }

    template<class T>
    typename boost::enable_if<boost::is_same<T, std::string>, T>::type
    read()
    {
        return readString<std::string>();
    }
    
    std::string readShortString()
    {
        boost::uint8_t len = read<boost::uint8_t>();
        const char * oldPos = pos_;
        pos_ += len;
        BOOST_ASSERT(pos_ <= end_);
        return std::string(oldPos, pos_);
    }

    void read(std::string & out)
    {
        boost::uint16_t len = read<boost::uint16_t>();
        const char * oldPos = pos_;
        pos_ += len;
        BOOST_ASSERT(pos_ <= end_);
        out.assign(oldPos, pos_);
    }

    template<class T, size_t n>
    void read(boost::array<T, n> & out)
    {
        for(size_t i = 0; i != n; ++i)
            out[i] = read<T>();
    }

    template<class T>
    void read(std::vector<T> & result)
    {
        boost::uint16_t len = read<boost::uint16_t>(); // check length
        result.reserve(len);
        while(result.size() != len)
            result.push_back(read<T>());
    }

    template<class T>
    typename boost::enable_if<detail::IsVector<T>, T>::type
    read()
    {
        T result;
        read<T>(result);
        return result;
    }

    template<class T>
    T readString()
    {
        boost::uint16_t len = read<boost::uint16_t>();
        const char * oldPos = pos_;
        pos_ += len;
        BOOST_ASSERT(pos_ <= end_);
        return T(oldPos, pos_);
    }

	template<class Len>
	std::wstring readUTFString(Len len)
    {
		const char * begin = pos_;
		pos_ += len;
		return mstd::deutf8(begin, len);
	}

    template<class Len>
    std::wstring readLenUTFString()
    {
        auto len = read<Len>();
		return readUTFString(len);
    }

    template<class T>
    T readWString()
    {
        boost::uint16_t len = read<boost::uint16_t>();
        T result;
        result.reserve(len);
        while(len--)
            result.push_back(read<uint16_t>());
        return result;
    }

    std::string readCString()
    {
        const char * p = pos_;
        const char * end = p + strlen(p);
        pos_ = end + 1;
        return std::string(p, end);
    }

    std::wstring readCUTFString()
    {
        const char * p = pos_;
        const char * end = p + strlen(p);
        pos_ = end + 1;
        return mstd::deutf8(p, end - p);
    }

    void readCString(std::string & result)
    {
        const char * p = pos_;
        const char * end = p + strlen(p);
        pos_ = end + 1;
        result.assign(p, end);
    }

    void readCString(std::string & result, size_t max)
    {
        const char * p = pos_;
        const char * end = std::find(p, p + max, 0);
        if(*end)
            throw InvalidStringException();
        pos_ = end + 1;
        result.assign(p, end);
    }

    void checkedReadCString(std::string & result, size_t max)
    {
        const char * p = pos_;
        const char * q = (std::min)(end_, p + max);
        const char * end = std::find(p, q, 0);
        if(end == end_)
            throw ReaderUnderflowException();
        if(*end)
            throw InvalidStringException();
        pos_ = end + 1;
        result.assign(p, end);
    }

    std::wstring readWCString()
    {
        const uint16_t * p = mstd::pointer_cast<const uint16_t*>(pos_);
        const uint16_t * end = p;
        while(*end)
            ++end;
        pos_ = mstd::pointer_cast<const char*>(end + 1);
        return std::wstring(p, end);
    }

    std::wstring readWCString(size_t limit)
    {
        const uint16_t * p = mstd::pointer_cast<const uint16_t*>(pos_);
        const uint16_t * end = std::find(p, p + limit, 0);
        pos_ = mstd::pointer_cast<const char*>(end + 1);
        return std::wstring(p, end);
    }

    template<class Elem>
    typename boost::enable_if<boost::mpl::or_<boost::is_same<Elem, char>, boost::is_same<Elem, unsigned char> >, void>::type
    readArray(std::vector<Elem> & out, size_t len)
    {
        out.resize(len);
        if(len)
        {
            memcpy(&out[0], pos_, len);
            pos_ += len;
        }
    }

    template<class Container>
    typename boost::enable_if<boost::is_pod<typename Container::value_type>, void>::type
    readArray(Container & container)
    {
        size_t len = container.size() * sizeof(typename Container::value_type);
        memcpy(&container[0], pos_, len);
        pos_ += len;
    }

    template<class Container>
    typename boost::enable_if<boost::is_pod<typename Container::value_type>, void>::type
    checkedReadArray(Container & container)
    {
        size_t len = container.size() * sizeof(typename Container::value_type);
        if(pos_ + len > end_)
            throw ReaderUnderflowException();
        memcpy(&container[0], pos_, len);
        pos_ += len;
    }

    const char * raw() const
    {
        return pos_;
    }

    const char * end() const
    {
        return end_;
    }

    const char * marked() const
    {
        return marked_;
    }
private:
    const char * pos_;
    const char * end_;
    const char * marked_;
};

class Buffer;

// std::string decompress(PacketReader<true> & reader);
NEXUS_DECL std::string decompress(const char * input, size_t size);
inline std::string decompress(const std::vector<char> & input) { return input.empty() ? std::string() : decompress(&input[0], input.size()); }
NEXUS_DECL std::string decompress(const Buffer & input);

NEXUS_DECL void decompress(const void * input, size_t size, std::vector<char> & out);

template<class SizeT, bool network, class Functor>
void processPackets(std::vector<char> & buffer, size_t & position, const Functor & functor)
{
    PacketReader reader(&buffer[0], position);
    while(reader.left() >= 1 + sizeof(SizeT))
    {
        uint8_t code = reader.read<uint8_t>();
        SizeT len = network ? mstd::ntoh(reader.read<SizeT>()) : reader.read<SizeT>();
        if(len > reader.left())
        {
            reader.revert(1 + sizeof(SizeT));
            break;
        }

        functor(code, PacketReader(reader.raw(), len));
        reader.skip(len);
    }

    if(reader.left() != position)
    {
        memmove(&buffer[0], reader.raw(), reader.left());
        position = reader.left();
    }
}

}
