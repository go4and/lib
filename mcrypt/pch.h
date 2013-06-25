/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include <algorithm>
#include <string>
#include <vector>

#include <boost/version.hpp>

#include <boost/aligned_storage.hpp>
#include <boost/assert.hpp>
#include <boost/array.hpp>
#include <boost/crc.hpp>
#include <boost/function.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include <boost/scope_exit.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/static_assert.hpp>

#include <boost/filesystem/path.hpp>
#if !_STLP_NO_IOSTREAMS
#include <boost/filesystem/fstream.hpp>
#endif

#include <boost/move/move.hpp>

#include <boost/mpl/or.hpp>

#include <boost/parameter/name.hpp>
#include <boost/parameter/preprocessor.hpp>

#include <openssl/bio.h>
#include <openssl/blowfish.h>
#include <openssl/bn.h>
#include <openssl/buffer.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/md5.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>

#include <mstd/cstdint.hpp>
#include <mstd/enum_utils.hpp>
#include <mstd/handle_base.hpp>
#include <mstd/hton.hpp>
#include <mstd/itoa.hpp>
#include <mstd/pointer_cast.hpp>
#include <mstd/rc_buffer.hpp>
#include <mstd/reference_counter.hpp>
#include <mstd/strings.hpp>

#define MCRYPT_BUILDING
