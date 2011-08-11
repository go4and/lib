#include "pch.h"

#include "SHA.h"

namespace mcrypt {

BOOST_STATIC_ASSERT(SHA_DIGEST_LENGTH == SHADigest::static_size);

class SHA::Context {
public:
    SHA_CTX value_;
};

SHA::SHA()
    : context_(new Context())
{
    SHA1_Init(&context_->value_);
}

SHA::~SHA()
{}

void SHA::feed(const void * src, size_t len)
{
    SHA1_Update(&context_->value_, src, len);
}

void SHA::feed(const std::vector<unsigned char> & src)
{
    SHA1_Update(&context_->value_, &src[0], src.size());
}

void SHA::feed(const std::vector<char> & src)
{
    feed(&src[0], src.size());
}

SHADigest SHA::digest()
{
    SHADigest result;
    SHA1_Final(result.c_array(), &context_->value_);
    return result;
}

void SHA::digest(SHADigest & out)
{
    SHA1_Final(out.c_array(), &context_->value_);
}

//////////////////////////////////////////////////////////////////////////
// Utility functions
//////////////////////////////////////////////////////////////////////////

#if !_STLP_NO_IOSTREAMS
template<class Path>
boost::optional<SHADigest> shaFileImpl(const Path & path)
{
    SHA sha;
    boost::filesystem::ifstream inf(path, std::ios::binary);

    if(!inf)
        return boost::optional<SHADigest>();

    char buffer[0x1000];
    while(inf)
    {
        inf.read(buffer, sizeof(buffer));
        sha.feed(buffer, static_cast<size_t>(inf.gcount()));
    }

    return sha.digest();
}

boost::optional<SHADigest> shaFile(const std::wstring & filename)
{
    return shaFile(boost::filesystem::wpath(filename));
}

#if BOOST_VERSION < 104600
boost::optional<SHADigest> shaFile(const boost::filesystem::wpath & path)
{
    return shaFileImpl(path);
}
#endif

boost::optional<SHADigest> shaFile(const boost::filesystem::path & path)
{
    return shaFileImpl(path);
}
#endif

SHADigest shaString(const std::string & str)
{
    mcrypt::SHA sha;
    sha.feed(str.c_str(), str.length());
    SHADigest digest;
    sha.digest(digest);
    return digest;
}

SHADigest shaBuffer(const void * data, size_t len)
{
    mcrypt::SHA sha;
    sha.feed(data, len);
    SHADigest digest;
    sha.digest(digest);
    return digest;
}

}
