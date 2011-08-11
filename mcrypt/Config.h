#pragma once

#include <boost/config.hpp>

#if defined(BOOST_HAS_DECLSPEC) && (MCRYPT_SHARED)
#   if defined(MCRYPT_BUILDING)
#       define MCRYPT_DECL __declspec(dllexport)
#   else
#       define MCRYPT_DECL __declspec(dllimport)
#   endif
#   if defined(_MSC_VER)
#       pragma warning(disable: 4251)
#       pragma warning(disable: 4275)
#   endif
#else
#   define MCRYPT_DECL
#endif
