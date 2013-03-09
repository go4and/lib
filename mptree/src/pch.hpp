#ifdef _MSC_VER
#pragma once
#endif

#include <vector>

#include <boost/array.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/optional.hpp>
#include <boost/unordered_set.hpp>

#include <boost/container/stable_vector.hpp>

#include <boost/type_traits/is_base_of.hpp>
#include <boost/type_traits/is_enum.hpp>
#include <boost/type_traits/is_integral.hpp>

#include <boost/utility/enable_if.hpp>

#include <yajl/yajl_parse.h>

#include <mstd/handle_base.hpp>
#include <mstd/itoa.hpp>
#include <mstd/pointer_cast.hpp>
#include <mstd/reference_counter.hpp>
#include <mstd/strings.hpp>

#define MPTREE_BUILDING 1
