#if defined(_MSC_VER)
#pragma once
#endif

#include <string.h>

#include <algorithm>
#include <deque>
#include <exception>
#include <string>
#include <vector>

#include <boost/config.hpp>

#include <boost/aligned_storage.hpp>
#include <boost/array.hpp>
#include <boost/assert.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/noncopyable.hpp>

#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/tcp.hpp>

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

#include <zlib.h>

#include <mstd/atomic.hpp>
#include <mstd/buffers.hpp>
#include <mstd/cstdint.hpp>
#include <mstd/enum_utils.hpp>
#include <mstd/exception.hpp>
#include <mstd/itoa.hpp>
#include <mstd/hton.hpp>
#include <mstd/pointer_cast.hpp>
#include <mstd/reference_counter.hpp>
#include <mstd/singleton.hpp>
#include <mstd/threads.hpp>

#include <mlog/Logging.h>
#include <mlog/ThreadTrace.h>

#include <mcrypt/MD5.h>
#include <mcrypt/Utils.h>

#if BOOST_WINDOWS

#include <boost/bind/protect.hpp>

#include <boost/thread/thread.hpp>

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/windows/overlapped_ptr.hpp>
#include <boost/asio/windows/stream_handle.hpp>

#endif

#define NEXUS_BUILDING
