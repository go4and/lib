#pragma once

#include <boost/config.hpp>

#if defined(BOOST_HAS_DECLSPEC) && (MLOG_SHARED)
#   if defined(MLOG_BUILDING)
#       define MLOG_DECL __declspec(dllexport)
#   else
#       define MLOG_DECL __declspec(dllimport)
#   endif
#else
#   define MLOG_DECL
#endif
