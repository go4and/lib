#include "pch.h"

#include "PacketWriter.h"

MLOG_DECLARE_LOGGER(packet_writer);

namespace nexus {

size_t compressSize(size_t len)
{
    return compressBound(len);
}

size_t compressSize(const char * begin, const char * end)
{
    return compressBound(end - begin);
}

std::string compress(const std::string & input)
{
    size_t len = input.length();
    size_t size = compressSize(input.length());
    char * out = static_cast<char*>(alloca(size));
    size = compress(input.c_str(), input.c_str() + len, out, size);
    return std::string(out, out + size);
}

size_t compress(const char * begin, const char * end, char * out, size_t outSize)
{
    z_stream stream;
    memset(&stream, 0, sizeof(stream));
    stream.next_in = mstd::pointer_cast<Bytef*>(const_cast<char*>(begin));
    stream.avail_in = end - begin;
    
    stream.next_out = mstd::pointer_cast<Bytef*>(out);
    stream.avail_out = outSize;
    
    if(deflateInit(&stream, 1) != Z_OK)
    {
        MLOG_MESSAGE(Error, "deflateInit failed");
        return 0;
    }

    if(deflate(&stream, Z_FINISH) != Z_STREAM_END) 
    {
        MLOG_MESSAGE(Error, "deflate failed");
        return 0;
    }

    if(deflateEnd(&stream) != Z_OK)
    {
        MLOG_MESSAGE(Error, "deflateEnd failed");
        return 0;
    }

    return stream.total_out;
}

}
