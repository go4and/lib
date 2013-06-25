/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

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
