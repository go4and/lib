#pragma once

#include <boost/config.hpp>

#if defined(BOOST_HAS_DECLSPEC) && (WXUTILS_SHARED)
#   if defined(BUILDING_WXUTILS)
#       define WXUTILS_DECL __declspec(dllexport)
#   else
#       define WXUTILS_DECL __declspec(dllimport)
#   endif
#else
#   define WXUTILS_DECL
#endif
