/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#if defined(_MSC_VER)
#pragma once
#endif

#if !MLOG_NO_LOGGING

#if defined(_MSC_VER)
#pragma warning(disable: 4251)
#pragma warning(disable: 4275)
#endif

#include <boost/config.hpp>

#if defined(BOOST_WINDOWS)
#include <Windows.h>
#include <ShlObj.h>
#else
#include <sys/time.h>
#endif

#include <string.h>

#ifdef ANDROID
#include <asm/page.h>
#include <android/log.h>
#endif

#include <exception>
#include <functional>
#include <iosfwd>
#include <iomanip>
#include <fstream>
#include <ostream>
#include <sstream>
#include <unordered_map>
#include <vector>

#include <boost/array.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/stream.hpp>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <boost/ptr_container/ptr_vector.hpp>

#include <boost/thread.hpp>

#include "Config.h"

#include <mstd/atomic.hpp>

#if MLOG_USE_BUFFERS
#include <mstd/buffers.hpp>
#else
#include <mstd/rc_buffer.hpp>
#endif

#include <mstd/cstdint.hpp>
#include <mstd/exception.hpp>
#include <mstd/filesystem.hpp>
#include <mstd/itoa.hpp>
#include <mstd/pointer_cast.hpp>
#include <mstd/reference_counter.hpp>
#include <mstd/reverse_lock.hpp>
#include <mstd/singleton.hpp>
#include <mstd/strings.hpp>
#include <mstd/threads.hpp>
#include <mstd/utf8.hpp>

#else

#include <boost/filesystem/path.hpp>

#endif

#define MLOG_BUILDING
