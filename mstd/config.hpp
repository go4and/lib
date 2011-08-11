#pragma once

#include <boost/config.hpp>

#if defined(BOOST_HAS_DECLSPEC) && (MSTD_SHARED)
#   if defined(MSTD_BUILD)
#       define MSTD_DECL __declspec(dllexport)
#   else
#       define MSTD_DECL __declspec(dllimport)
#   endif
#   if defined(_MSC_VER)
#       pragma warning(disable: 4251)
#       pragma warning(disable: 4275)
#   endif
#else
#   define MSTD_DECL
#endif

#if BOOST_WINDOWS
#define MSTD_STDCALL __stdcall
#else
#define MSTD_STDCALL
#endif
