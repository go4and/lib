/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

#if !defined(BUILDING_WXUTILS)
#include <boost/cast.hpp>
#endif

namespace wxutils {

template<class T, class Parent>
class Any : public Parent {
public:
    Any(const T & t)
        : t_(t) {}

    const T & value() const
    {
        return t_;
    }
private:
    T t_;
};

template<class P, class T>
Any<T, P> * any(const T & t)
{
    return new Any<T, P>(t);
}

template<class T, class P>
T any_cast(P * object)
{
    return boost::polymorphic_downcast<Any<T, P>*>(object)->value();
}

}
