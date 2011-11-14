#pragma once

#include <boost/config.hpp>

#if !defined(CALC_SHARED)
#   if defined(NDEBUG)
#       define CALC_SHARED 0
#   else
#       define CALC_SHARED 1
#   endif
#endif

#if defined(BOOST_HAS_DECLSPEC) && (CALC_SHARED)
#   if defined(BUILDING_CALC)
#       define CALC_DECL __declspec(dllexport)
#   else
#       define CALC_DECL __declspec(dllimport)
#   endif
#else
#   define CALC_DECL
#endif
