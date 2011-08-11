#pragma once

#include "config.hpp"

namespace mstd {

#if defined(_MSC_VER)
typedef unsigned long thread_id;
#elif defined(__APPLE__)
typedef size_t thread_id; 
#else
typedef unsigned long int thread_id;
#endif

MSTD_DECL thread_id this_thread_id();
MSTD_DECL size_t count_process_threads();

}
