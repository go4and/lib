#ifdef _MSC_VER
#pragma once
#endif

#include <vector>

#include <boost/array.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/optional.hpp>

#include <boost/mpl/has_xxx.hpp>

#include <boost/type_traits/is_base_of.hpp>
#include <boost/type_traits/is_enum.hpp>
#include <boost/type_traits/is_integral.hpp>

#include <boost/utility/enable_if.hpp>

#include <mstd/itoa.hpp>
#include <mstd/reference_counter.hpp>
#include <mstd/strings.hpp>

#define MPTREE_BUILDING 1
