/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include "pch.h"

#include "PacketPacker.h"

namespace nexus {

size_t StringPacker::packSize(const char * input)
{
    return strlen(input) + 2;
}

void StringPacker::pack(char *&out, const char *input)
{
    size_t len = strlen(input);
    write<boost::uint16_t>(out, len);
    memcpy(out, input, len);
    out += len;
}

size_t StringPacker::packSize(const std::string &input)
{
    return input.length() + 2;
}

void StringPacker::pack(char *& out, const std::string & input)
{
    size_t len = input.length();
    write<boost::uint16_t>(out, len);
    memcpy(out, input.c_str(), len);
    out += len;
}

size_t StringPacker::packSize(const StringWithLen & input)
{
    return input.len + 2;
}

void StringPacker::pack(char *& out, const StringWithLen & input)
{
    size_t len = input.len;
    write<boost::uint16_t>(out, len);
    memcpy(out, input.data, len);
    out += len;
}

size_t ShortStringRef::packSize() const
{
    return ref_->length() + 1;
}

void ShortStringRef::pack(char *& out) const
{
    size_t len = ref_->length();
    write<boost::uint8_t>(out, len);
    memcpy(out, ref_->c_str(), len);
    out += len;
}

size_t CStringPacker::packSize(CString<char> input)
{
    return strlen(input.get()) + 1;
}

void CStringPacker::pack(char *& out, CString<char> input)
{
    size_t len = strlen(input.get()) + 1;
    memcpy(out, input.get(), len);
    out += len;
}

size_t CStringPacker::packSize(CStringRef<char> input)
{
    return input.get().length() + 1;
}

void CStringPacker::pack(char *& out, CStringRef<char> input)
{
    size_t len = input.get().length() + 1;
    memcpy(out, input.get().c_str(), len);
    out += len;
}

/*
size_t CStringPacker::packSize(const wchar_t * input)
{
    return wcslen(input) * 2 + 2;
}

void StringPacker::pack(char *&out, const wchar_t *input)
{
    BOOST_STATIC_ASSERT(sizeof(wchar_t) == 2);

    size_t len = wcslen(input) * 2 + 2;
    memcpy(out, input, len);
    out += len;
}*/

size_t CStringPacker::packSize(CStringRef<wchar_t> input)
{
    return input.get().length() * 2 + 2;
}

namespace {

template<bool fast>
class CStringPackerHelper;

template<>
class CStringPackerHelper<true> {
public:
    static inline void apply(char *& out, const std::wstring & input)
    {
        size_t len = input.length() * 2 + 2;
        memcpy(out, input.c_str(), len);
        out += len;
    }
};

template<>
class CStringPackerHelper<false> {
public:
    static inline void apply(char *& out, const std::wstring & input)
    {
        size_t len = input.length() + 1;
        for(auto i = input.c_str(), end = i + len + 1; i != end; ++i)
        {
            memcpy(out, i, 2);
            out += 2;
        }
    }
};

}

void CStringPacker::pack(char *& out, CStringRef<wchar_t> input)
{
    CStringPackerHelper<sizeof(wchar_t) == 2>::apply(out, input.get());
}

size_t RawDataPacker::packSize(const std::pair<const char*, const char*> & p)
{
    return p.second - p.first;
}

void RawDataPacker::pack(char *& pos, const std::pair<const char*, const char*> & p)
{
    size_t len = p.second - p.first;
#ifdef NDEBUG        
    memcpy(pos, p.first, len);
#else
    std::copy(p.first, p.second, pos);
#endif
    pos += len;
}

BufferAsString::BufferAsString(const nexus::Buffer &value)
    : value_(value) {}

size_t BufferAsString::packSize() const
{
    return value_.size() + 2;
}

void BufferAsString::pack(char *& out) const
{
    size_t len = value_.size();
    write<boost::uint16_t>(out, len);
    memcpy(out, value_.data(), len);
    out += len;
}

void badExpectedSize(PacketCode code, size_t expectedSize, size_t size)
{
    throw InvalidPackSizeException() << PacketCodeInfo(code) << ExpectedSizeInfo(expectedSize) << FoundSizeInfo(size - 1); \
}

}
