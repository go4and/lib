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

#define BOOST_BIND_NO_PLACEHOLDERS

#include <boost/config.hpp>

#ifndef BOOST_WINDOWS
#include <grp.h>
#include <pwd.h>
#include <sys/types.h>
#endif

#include <string.h>

#include <algorithm>
#include <deque>
#include <exception>
#include <queue>
#include <string>
#include <unordered_set>
#include <vector>

#include <boost/aligned_storage.hpp>
#include <boost/array.hpp>
#include <boost/assert.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/noncopyable.hpp>

#include <boost/asio/buffer.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/generic/datagram_protocol.hpp>
#include <boost/asio/ip/icmp.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/unicast.hpp>
#include <boost/asio/local/stream_protocol.hpp>

#include <boost/date_time/posix_time/posix_time_io.hpp>

#include <boost/mpl/bool.hpp>
#include <boost/mpl/find_if.hpp>
#include <boost/mpl/vector.hpp>

#include <boost/preprocessor/expand.hpp>
#include <boost/preprocessor/expr_if.hpp>
#include <boost/preprocessor/if.hpp>
#include <boost/preprocessor/repeat_from_to.hpp>

#include <boost/preprocessor/repetition/enum.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing.hpp>
#include <boost/preprocessor/repetition/enum_trailing_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>

#include <boost/ptr_container/ptr_vector.hpp>

#include <boost/range/iterator_range.hpp>

#include <boost/system/error_code.hpp>

#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>

#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_pod.hpp>
#include <boost/type_traits/is_same.hpp>

#include <boost/unordered/unordered_set_fwd.hpp>

#include <boost/utility/enable_if.hpp>
#include <boost/utility/in_place_factory.hpp>

#include <zlib.h>

#include <mstd/atomic.hpp>
#include <mstd/buffers.hpp>
#include <mstd/command_queue.hpp>
#include <mstd/cstdint.hpp>
#include <mstd/enum_utils.hpp>
#include <mstd/exception.hpp>
#include <mstd/itoa.hpp>
#include <mstd/hton.hpp>
#include <mstd/null.hpp>
#include <mstd/pointer_cast.hpp>
#include <mstd/rc_buffer.hpp>
#include <mstd/reference_counter.hpp>
#include <mstd/singleton.hpp>
#include <mstd/threads.hpp>
#include <mstd/tid_map.hpp>
#include <mstd/utf8.hpp>

#include <mlog/Dumper.h>
#include <mlog/Logging.h>
#include <mlog/ThreadTrace.h>
#include <mlog/Utils.h>

#include <mcrypt/MD5.h>
#include <mcrypt/Utils.h>

#if BOOST_WINDOWS

#include <boost/thread/thread.hpp>

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/windows/overlapped_ptr.hpp>
#include <boost/asio/windows/stream_handle.hpp>

#endif

#define NEXUS_BUILDING

using namespace std::placeholders;
