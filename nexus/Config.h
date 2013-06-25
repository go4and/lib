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

#if defined(BOOST_HAS_DECLSPEC) && (NEXUS_SHARED)
#   if defined(NEXUS_BUILDING)
#       define NEXUS_DECL __declspec(dllexport)
#   else
#       define NEXUS_DECL __declspec(dllimport)
#   endif
#else
#   define NEXUS_DECL
#endif
