#ifndef MSTD_MACHINE_COMMON_PROCESSING
#error Dont include this file directly, use common.hpp
#endif

#include "linux_common.hpp"

namespace mstd { namespace detail {

MSTD_DEFINE_ATOMICS(1, "")
MSTD_DEFINE_ATOMICS(2, "")
MSTD_DEFINE_ATOMICS(4, "")
MSTD_DEFINE_ATOMICS(8, "q")

#undef MSTD_DEFINE_ATOMICS

} }
