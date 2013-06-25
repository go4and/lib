/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#size_t CStringPacker::packSize(CStringRef<wchar_t> input)
{
    return input.get().length() * 2 + 2;
}

void CStringPacker::pack(char *& out, CStringRef<wchar_t> input)
{
    BOOST_STATIC_ASSERT(sizeof(wchar_t) == 2);

    size_t len = input.get().length() * 2 + 2;
    memcpy(out, input.get().c_str(), len);
    out += len;
}

#endif

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
