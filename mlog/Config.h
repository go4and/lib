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

#if !defined(MLOG_USE_BUFFERS)
#  if defined(ANDROID)
#    define MLOG_USE_BUFFERS 0
#  else
#    define MLOG_USE_BUFFERS 1
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

