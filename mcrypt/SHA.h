#pragma once

#ifndef MCRYPT_BUILDING
#include <vector>

#include <boost/aligned_storage.hpp>
#endif

#include "Config.h"

#include "Defines.h"
#include "Hasher.h"

namespace mcrypt {

MCRYPT_HASHER(SHA1, sha1, 20);

SHA1Digest fromBase64(const std::string & string);

MCRYPT_HASHER(SHA256, sha256, 32);

}
