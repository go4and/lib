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

#if defined(BOOST_HAS_DECLSPEC) && (MLOG_SHARED)
#   if defined(MLOG_BUILDING)
#       define MLOG_DECL __declspec(dllexport)
#   else
#       define MLOG_DECL __declspec(dllimport)
#   endif
#else
#   define MLOG_DECL
#endif

#if !defined(MLOG_USE_BUFFERS)
#  if defined(ANDROID)
#    define MLOG_USE_BUFFERS 0
#  else
#    define MLOG_USE_BUFFERS 1
#  endif
#endif

#if !defined(MLOG_USE_MARKUP)
#  if !BOOST_WINDOWS && !defined(__APPLE__) && !defined(ANDROID)
#    define MLOG_USE_MARKUP 1
#  else
#    define MLOG_USE_MARKUP 0
#  endif
#endif

#if MLOG_USE_BUFFERS
#include <mstd/buffers.hpp>
namespace mlog {
    typedef mstd::pbuffer Buffer;

    inline char * bufferData(const Buffer & buffer)
    {
        return buffer->data();
    }
}
#else
#include <mstd/rc_buffer.hpp>
namespace mlog {
    typedef mstd::rc_buffer Buffer;

    inline char * bufferData(const Buffer & buffer)
    {
        return buffer.data();
    }
}
#endif

