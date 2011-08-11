#pragma once

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
