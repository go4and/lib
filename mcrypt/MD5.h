#pragma once

#ifndef MCRYPT_BUILDING
#include <vector>

#include <boost/array.hpp>
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>

#include <boost/filesystem/path.hpp>
#endif

#include "Config.h"

#include "Defines.h"

namespace mcrypt {
    
class MCRYPT_DECL MD5 {
public:
    MD5();
    ~MD5();
    
    void feed(const unsigned char * src, size_t len);
    void feed(const char * src, size_t len);

    void feed(const std::string & str) { feed(str.c_str(), str.length()); }

    template<class Ch>
    typename boost::enable_if<boost::mpl::or_<boost::is_same<char, Ch>, boost::is_same<unsigned char, Ch> >, void>::type
    feed(const std::vector<Ch> & src) { if(!src.empty()) feed(&src[0], src.length()); }

    template<class Ch, size_t N>
    typename boost::enable_if<boost::mpl::or_<boost::is_same<char, Ch>, boost::is_same<unsigned char, Ch> >, void>::type
    feed(const boost::array<Ch, N> & src) { feed(src.data(), N); }

    MD5Digest digest();
    void digest(MD5Digest & out);
private:
    class Context;
    
    boost::scoped_ptr<Context> context_;
};

MCRYPT_DECL boost::optional<MD5Digest> md5File(const std::wstring & filename);
MCRYPT_DECL boost::optional<MD5Digest> md5File(const boost::filesystem::wpath & path);
MCRYPT_DECL boost::optional<MD5Digest> md5File(const boost::filesystem::path & path);

MCRYPT_DECL MD5Digest md5String(const std::string & input);
MCRYPT_DECL MD5Digest md5Buffer(const void * buffer, size_t len);
    
}
