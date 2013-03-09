#pragma once

#ifndef MCRYPT_BUILDING
#include <boost/optional.hpp>

#include <boost/filesystem/path.hpp>
#endif

namespace mcrypt {

#if !_STLP_NO_IOSTREAMS
template<class Engine>
boost::optional<typename Engine::result_type> hashFile(const boost::filesystem::path & path)
{
    boost::filesystem::ifstream inf(path, std::ios::binary);
    Engine engine;

    if(!inf)
        return boost::optional<typename Engine::result_type>();

    char buffer[0x400];
    while(inf)
    {
        inf.read(buffer, sizeof(buffer));
        engine.feed(buffer, static_cast<size_t>(inf.gcount()));
    }

    return engine.digest();
}
#endif

template<class Engine>
typename Engine::result_type hashBuffer(const void * data, size_t len)
{
    Engine engine;
    engine.feed(data, len);
    return engine.digest();
}

template<class Engine>
typename Engine::result_type hashString(const std::string & input)
{
    return hashBuffer<Engine>(input.c_str(), input.length());
}

template<class Derived, class Result>
class HashEngine {
public:
    typedef Result result_type;

    void feed(const std::vector<char> & src) { self()->feed(&src[0], src.size); }
    void feed(const std::vector<unsigned char> & src) { self()->feed(&src[0], src.size); }

    Result digest()
    {
        typename Derived::result_type result;
        self()->digest(result);
        return result;
    }
private:
    Derived * self() { return static_cast<Derived*>(this); }
};

}
