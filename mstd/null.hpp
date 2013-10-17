/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
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
