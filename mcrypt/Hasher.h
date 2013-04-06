#pragma once

#ifndef MCRYPT_BUILDING
#include <boost/optional.hpp>

#include <boost/filesystem/fstream.hpp>
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

    return engine.final();
}
#endif

template<class Engine, class Data>
typename Engine::result_type hashBuffer(const Data * data, size_t len)
{
    Engine engine;
    engine.update(data, len);
    return engine.final();
}

template<class Engine>
typename Engine::result_type hashString(const std::string & input)
{
    return hashBuffer<Engine>(input.c_str(), input.length());
}

class HasherDescriptor {
public:
    explicit HasherDescriptor(const void * evp)
        : evp_(evp)
    {
    }

    const void * handle() const { return evp_; }
private:
    const void * evp_;
};

class GenericHasher : boost::noncopyable {
public:
    explicit GenericHasher(const HasherDescriptor & descriptor);
    ~GenericHasher();

    void update(const char * data, size_t size);
    inline void update(const unsigned char * data, size_t size) { update(mstd::pointer_cast<const char*>(data), size); }
    inline void update(const std::vector<char> & src) { update(&src[0], src.size()); }
    inline void update(const std::vector<unsigned char> & src) { update(mstd::pointer_cast<const char*>(&src[0]), src.size()); }
    
    size_t final(unsigned char * out);
private:
    typedef boost::aligned_storage<6 * sizeof(void*)> Context;
    Context context_;
};

template<class Tag, size_t digestLen>
class Hasher : public GenericHasher {
public:
    typedef boost::array<unsigned char, digestLen> result_type;

    static HasherDescriptor descriptor() { return HasherDescriptor(Tag::evp()); }

    Hasher()
        : GenericHasher(descriptor())
    {
    }

    result_type final()
    {
        result_type result;
        final(result);
        return result;
    }
    
    void final(result_type & out)
    {
        BOOST_VERIFY(GenericHasher::final(&out[0]) == out.size());
    }
};

#define MCRYPT_HASHER(name, lname, digestlen) \
    struct BOOST_PP_CAT(name, Tag) { static const void * evp(); }; \
    typedef Hasher<BOOST_PP_CAT(name, Tag), digestlen> name; \
    typedef name::result_type BOOST_PP_CAT(name, digest); \
    inline name::result_type BOOST_PP_CAT(lname, String(const std::string & input)) \
    { \
        return hashString<name>(input); \
    } \
    template<class Data> \
    inline name::result_type BOOST_PP_CAT(lname, Buffer)(const Data * data, size_t len) \
    { \
        return hashBuffer<name>(data, len); \
    } \
    template<class Data> \
    inline name::result_type BOOST_PP_CAT(lname, Buffer)(const Data * begin, const Data * end) \
    { \
        return hashBuffer<name>(begin, end - begin); \
    } \
    /**/

}
