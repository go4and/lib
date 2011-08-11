#pragma once

#include <boost/type_traits/remove_reference.hpp>

namespace mstd {

template<class T>
typename boost::remove_reference<T>::type *
null()
{
    return 0;
}

}
