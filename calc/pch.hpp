#ifdef _MSC_VER
#pragma once

#pragma warning(disable: 4251)
#pragma warning(disable: 4275)
#endif

#if defined(I3D_OS_S3E)
#include <boost/config.hpp>
#undef BOOST_NO_INTRINSIC_WCHAR_T
#endif

#include <exception>
#include <string>
#include <vector>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <boost/algorithm/string/case_conv.hpp>

#include <boost/fusion/adapted/struct/adapt_struct.hpp>

#include <boost/type_traits/function_traits.hpp>

#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>

#include <boost/unordered_map.hpp>

#include <boost/spirit/home/qi.hpp>
#include <boost/spirit/home/support.hpp>

#include <boost/spirit/home/phoenix.hpp>

#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/remove_pointer.hpp>

#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/get.hpp>

#include <mstd/exception.hpp>
#include <mstd/null.hpp>
#include <mstd/phoenix.hpp>
#include <mstd/singleton.hpp>

#define BUILDING_CALC
