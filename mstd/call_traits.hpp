#pragma once

#include <boost/type_traits/is_pod.hpp>

#include <boost/mpl/and.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/or.hpp>

namespace mstd {

template <typename T>
class call_traits {
public:
   typedef T value_type;
   typedef T& reference;
   typedef const T& const_reference;
   typedef typename boost::mpl::if_<
                        boost::mpl::and_<boost::mpl::or_<boost::is_pod<T>, boost::is_enum<T> >, 
                                         boost::mpl::bool_<sizeof(T) <= sizeof(void*)> >,
                        T, const T&>::type param_type;
};

template <typename T>
struct call_traits<T&>
{
   typedef T& value_type;
   typedef T& reference;
   typedef const T& const_reference;
   typedef T& param_type;
};

}
