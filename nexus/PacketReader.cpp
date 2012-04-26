#include "pch.h"

#include "Buffer.h"

#include "PacketReader.h"

namespace nexus {

std::string decompress(const Buffer & input)
{
    return decompress(input.data(), input.size());
}

std::string decompress(const char * data, size_t size)
{
    std::string result;
    if(!size)
        return result;
    result.resize(size * 10); // TODO
    z_stream stream;
    memset(&stream, 0, sizeof(stream));
    stream.next_in = mstd::pointer_cast<Bytef*>(const_cast<char*>(data));
    stream.avail_in = size;
    
    stream.next_out = mstd::pointer_cast<Bytef*>(&result[0]);
    stream.avail_out = result.size();
    
    if(inflateInit(&stream) != Z_OK)
        throw InflateException();

    if(inflate(&stream, Z_FINISH) != Z_STREAM_END) 
        throw InflateException();

    if(inflateEnd(&stream) != Z_OK)
        throw InflateException();

    result.resize(stream.total_out);

    return result;
}

void decompress(const void * input, size_t size, std::vector<char> & result)
{
    result.clear();
    if(!size)
        return;
    result.resize(std::max(size * 10, result.capacity())); // TODO
    z_stream stream;
    memset(&stream, 0, sizeof(stream));
    stream.next_in = static_cast<Bytef*>(const_cast<void*>(input));
    stream.avail_in = size;
    
    stream.next_out = mstd::pointer_cast<Bytef*>(&result[0]);
    stream.avail_out = result.size();
    
    if(inflateInit(&stream) != Z_OK)
        throw InflateException();

    int code = inflate(&stream, Z_FINISH);
    if(code != Z_STREAM_END) 
        throw InflateException();

    if(inflateEnd(&stream) != Z_OK)
        throw InflateException();

    result.resize(stream.total_out);
}

}
