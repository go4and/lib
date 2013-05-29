#pragma once

#ifndef MCRYPT_BUILDING
#include <string.h>

#include <string>
#include <vector>

#include <boost/aligned_storage.hpp>
#include <boost/noncopyable.hpp>

#include <boost/parameter/name.hpp>
#include <boost/parameter/preprocessor.hpp>

#include <mstd/pointer_cast.hpp>
#include <mstd/rc_buffer.hpp>
#endif

namespace mstd {

class rc_buffer;

}

namespace mcrypt {

enum CipherMode {
    modeEncrypt,
    modeDecrypt,
};

class CipherDescriptor {
public:
    explicit CipherDescriptor(const void * evp)
        : evp_(evp)
    {
    }

    const void * handle() const { return evp_; }

    size_t ivSize() const;
    size_t blockSize() const;
    size_t keySize() const;
private:
    const void * evp_;
};

namespace keywords {
    BOOST_PARAMETER_NAME(encrypt)
    BOOST_PARAMETER_NAME(decrypt)
    BOOST_PARAMETER_NAME(key)
    BOOST_PARAMETER_NAME(keysize)
    BOOST_PARAMETER_NAME(iv)
    BOOST_PARAMETER_NAME(padding)
}

using namespace keywords;

inline const char * getBytes(const char * raw) { return raw; }
inline const char * getBytes(const unsigned char * raw) { return mstd::pointer_cast<const char*>(raw); }
inline const char * getBytes(const std::string & raw) { return raw.c_str(); }
inline const char * getBytes(const std::vector<char> & raw) { return &raw[0]; }
inline const char * getBytes(const std::vector<unsigned char> & raw) { return mstd::pointer_cast<const char*>(&raw[0]); }

inline size_t bytesSize(const std::string & raw) { return raw.length(); }
inline size_t bytesSize(const std::vector<char> & raw) { return raw.size(); }
inline size_t bytesSize(const std::vector<unsigned char> & raw) { return raw.size(); }
inline size_t bytesSize(const char * raw) { return strlen(raw); }

template<class Key>
class BytesSize {
public:
    typedef size_t result_type;

    explicit BytesSize(const Key & key)
        : key_(key)
    {
    }
    
    size_t operator()() const
    {
        return bytesSize(key_);
    }
private:
    const Key & key_;
};

template<class ArgumentPack>
class GetDecrypt {
public:
    typedef bool result_type;

    explicit GetDecrypt(const ArgumentPack & args)
        : args_(args)
    {
    }
    
    bool operator()() const
    {
        return !args_[_decrypt];
    }
private:
    const ArgumentPack & args_;
};

class GenericCipher : boost::noncopyable {
public:
    template<class ArgumentPack>
    GenericCipher(const CipherDescriptor & descriptor, const ArgumentPack & args)
    {
        typedef typename boost::parameter::binding<ArgumentPack, tag::encrypt, boost::mpl::void_>::type EncryptType;
        typedef typename boost::parameter::binding<ArgumentPack, tag::decrypt, boost::mpl::void_>::type DecryptType;
        BOOST_STATIC_ASSERT((boost::mpl::or_<boost::is_same<EncryptType, boost::mpl::void_>,
                                             boost::is_same<DecryptType, boost::mpl::void_> >::value));
        CipherMode mode = args[_encrypt || GetDecrypt<ArgumentPack>(args)] ? modeEncrypt : modeDecrypt;
        const char * key = getBytes(args[_key]);
        size_t keySize = args[_keysize || BytesSize<typename boost::parameter::binding<ArgumentPack, tag::key>::type>(args[_key])];
        const char * iv = getBytes(args[_iv | static_cast<const char*>(0)]);
        init(descriptor, mode, key, keySize, iv, args[_padding|true]);
    }
    
    ~GenericCipher();

    template<class C1, class C2>
    void process(std::vector<C1> & out, const std::vector<C2> & src)
    {
        if(src.empty())
            out.clear();
        else
            process(out, &src[0], src.size());
    }

    template<class C1, class C2>
    void process(std::vector<C1> & out, const C2 * begin, const C2 * end)
    {
        process(out, begin, end - begin);
    }

    template<class C1, class C2>
    void process(std::vector<C1> & out, const C2 * begin, size_t len)
    {
        if(!len)
            out.clear();
        else {
            out.resize(len + descriptor().blockSize());
            size_t outlen = process(&out[0], begin, len);
            BOOST_ASSERT(out.size() >= outlen);
            out.resize(outlen);
        }
    }

    template<class C2>
    void process(mstd::rc_buffer & out, const C2 * begin, size_t len)
    {
        BOOST_STATIC_ASSERT(sizeof(C2) == 1);
        if(!len)
            out.reset();
        else {
            out = mstd::rc_buffer(len + descriptor().blockSize());
            size_t outlen = process(out.data(), mstd::pointer_cast<const char*>(begin), len);
            BOOST_ASSERT(out.size() >= outlen);
            out.resize(outlen);
        }
    }

    void process(std::vector<unsigned char> & out, const unsigned char * begin, const unsigned char * end)
    {
        process(out, begin, end - begin);
    }

    void process(std::vector<unsigned char> & out, const unsigned char * begin, size_t len)
    {
        if(!len)
            out.clear();
        else {
            out.resize(len + descriptor().blockSize());
            size_t outlen = process(&out[0], begin, len);
            BOOST_ASSERT(out.size() >= outlen);
            out.resize(outlen);
        }
    }

    size_t process(char * out, const std::vector<char> & src)
    {
        if(!src.empty())
            return process(out, &src[0], src.size());
        else
            return 0;
    }

    size_t process(char * out, const std::string & src)
    {
        return process(out, src.c_str(), src.size());
    }

    inline size_t process(unsigned char * out, const unsigned char * begin, const unsigned char * end)
    {
        return process(mstd::pointer_cast<char*>(out), mstd::pointer_cast<const char*>(begin), end - begin);
    }

    inline size_t process(unsigned char * out, const unsigned char * begin, size_t len)
    {
        return process(mstd::pointer_cast<char*>(out), mstd::pointer_cast<const char*>(begin), len);
    }

    inline size_t process(char * out, const char * begin, const char * end)
    {
        return process(out, begin, end - begin);
    }
    
    size_t process(char * out, const char * input, size_t len)
    {
        size_t outlen = update(out, input, len);
        out += outlen;
        outlen += final(out);
        return outlen;
    }
    
    size_t update(char * out, const char * input, size_t len);
    size_t final(char * out);
private:
    CipherDescriptor descriptor();

    void init(const CipherDescriptor & descriptor, CipherMode mode, const char * key, size_t len, const char * ivec, bool padding);

#ifndef BOOST_WINDOWS
    typedef boost::aligned_storage<sizeof(void*) == 8 ? 168 : 140> Context;
#else
    typedef boost::aligned_storage<sizeof(void*) == 8 ? 160 : 140> Context;
#endif
    Context context_;
};

template<class Tag>
class CipherBase : public GenericCipher {
public:
    static CipherDescriptor descriptor() { return CipherDescriptor(Tag::evp()); }

    template<class ArgumentPack>
    explicit CipherBase(const ArgumentPack & pack)
        : GenericCipher(descriptor(), pack)
    {
    }
};

template<class Tag>
class Cipher : public CipherBase<Tag> {
public:
    BOOST_PARAMETER_CONSTRUCTOR(
        Cipher, (CipherBase<Tag>), tag,
            (required
                (key, *)
            )
            (optional
                (encrypt, (bool))
                (decrypt, (bool))
                (keysize, (size_t))
                (iv, *)
                (padding, (bool))
            )
        )
};

#define MCRYPT_CIPHER(name) \
    struct BOOST_PP_CAT(name, Tag) { static const void * evp(); }; \
    typedef Cipher<BOOST_PP_CAT(name, Tag)> name; \
    /**/

}
