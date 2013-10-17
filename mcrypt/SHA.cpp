/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include "pch.h"

#include "SHA.h"

namespace mcrypt {

BOOST_STATIC_ASSERT(SHA_DIGEST_LENGTH == SHA1Digest::static_size);

const void * SHA1Tag::evp() { return EVP_sha1(); }

BOOST_STATIC_ASSERT(SHA256_DIGEST_LENGTH == SHA256Digest::static_size);

const void * SHA256Tag::evp() { return EVP_sha256(); }

//////////////////////////////////////////////////////////////////////////
// Utility functions
//////////////////////////////////////////////////////////////////////////

}
