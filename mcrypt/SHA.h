#pragma once

#ifndef MCRYPT_BUILDING
#include <vector>

#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>

#include <boost/filesystem/path.hpp>
#endif

#include "Config.h"

#include "Defines.h"

namespace mcrypt {
    
class MCRYPT_DECL SHA {
public:
    SHA();
    ~SHA();
    
    void feed(const void * src, size_t len);
    void feed(const std::vector<char> & src);
    void feed(const std::vector<unsigned char> & src);

    SHADigest digest();
    void digest(SHADigest & out);
private:
    class Context;
    
    boost::scoped_ptr<Context> context_;
};

MCRYPT_DECL boost::optional<SHADigest> shaFile(const std::wstring & filename);
MCRYPT_DECL boost::optional<SHADigest> shaFile(const boost::filesystem::wpath & path);
MCRYPT_DECL boost::optional<SHADigest> shaFile(const boost::filesystem::path & path);

MCRYPT_DECL SHADigest shaString(const std::string & input);
MCRYPT_DECL SHADigest shaBuffer(const void * data, size_t len);
    
}
