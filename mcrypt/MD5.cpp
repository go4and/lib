#include "pch.h"

#include "MD5.h"

namespace mcrypt {

BOOST_STATIC_ASSERT(MD5_DIGEST_LENGTH == MD5Digest::static_size);

class MD5::Context {
public:
    MD5_CTX value_;
};

MD5::MD5()
    : context_(new Context())
{
    MD5_Init(&context_->value_);
}

MD5::~MD5()
{}

void MD5::feed(const unsigned char * src, size_t len)
{
    MD5_Update(&context_->value_, src, len);
}

void MD5::feed(const char * src, size_t len)
{
    feed(mstd::pointer_cast<const unsigned char*>(src), len);
}

MD5Digest MD5::digest()
{
    MD5Digest result;
    MD5_Final(result.c_array(), &context_->value_);
    return result;
}

void MD5::digest(MD5Digest & out)
{
    MD5_Final(out.c_array(), &context_->value_);
}

//////////////////////////////////////////////////////////////////////////
// Utility functions
//////////////////////////////////////////////////////////////////////////

#if !_STLP_NO_IOSTREAMS
template<class Path>
boost::optional<MD5Digest> md5FileImpl(const Path & path)
{
    MD5 md5;
    boost::filesystem::ifstream inf(path, std::ios::binary);

    if(!inf)
        return boost::optional<MD5Digest>();

    char buffer[0x1000];
    while(inf)
    {
        inf.read(buffer, sizeof(buffer));
        md5.feed(buffer, static_cast<size_t>(inf.gcount()));
    }

    return md5.digest();
}

boost::optional<MD5Digest> md5File(const std::wstring & filename)
{
    return md5File(boost::filesystem::wpath(filename));
}

#if BOOST_VERSION < 104600
boost::optional<MD5Digest> md5File(const boost::filesystem::wpath & path)
{
    return md5FileImpl(path);
}
#endif

boost::optional<MD5Digest> md5File(const boost::filesystem::path & path)
{
    return md5FileImpl(path);
}
#endif

MD5Digest md5String(const std::string & str)
{
    mcrypt::MD5 md5;
    md5.feed(str.c_str(), str.length());
    MD5Digest digest;
    md5.digest(digest);
    return digest;
}

MD5Digest md5Buffer(const void * buffer, size_t len)
{
    mcrypt::MD5 md5;
    md5.feed(static_cast<const unsigned char*>(buffer), len);
    MD5Digest digest;
    md5.digest(digest);
    return digest;
}

}
