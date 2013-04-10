#include "pch.h"

#include "crc32.h"

namespace mcrypt {

std::vector<char> crc32check(const std::vector<char> & src)
{
    BOOST_STATIC_ASSERT(sizeof(crc32_t) == 4);

    if(src.size() <= sizeof(crc32_t))
        return std::vector<char>();
    std::vector<char> result(src.begin(), src.end() - sizeof(crc32_t));
    crc32_t crc = ntohl(*reinterpret_cast<const crc32_t*>(&src[src.size() - sizeof(crc32_t)]));
    return (mcrypt::crc32(result) == crc) ? result : std::vector<char>();
}

void crc32append(std::vector<char> & data)
{
    BOOST_STATIC_ASSERT(sizeof(crc32_t) == 4);

    crc32_t crc = htonl(crc32(data));
    unsigned char * start = mstd::pointer_cast<unsigned char*>(&crc);
    data.insert(data.end(), start, start + sizeof(crc));
}

crc32_t crc32(const std::vector<char> & src)
{
    if(src.empty())
        return 0;
    else
        return boost::crc<32, 0x04C11DB7, 0, 0, false, false>(&src[0], src.size());
}

crc32_t crc32(const char * begin, const char * end)
{
    return boost::crc<32, 0x04C11DB7, 0, 0, false, false>(begin, end - begin);
}

crc32_t crc32(const std::vector<unsigned char> & src)
{
    if(src.empty())
        return 0;
    else
        return boost::crc<32, 0x04C11DB7, 0, 0, false, false>(&src[0], src.size());
}

crc32_t crc32(const unsigned char * begin, const unsigned char * end)
{
    return boost::crc<32, 0x04C11DB7, 0, 0, false, false>(begin, end - begin);
}

}
