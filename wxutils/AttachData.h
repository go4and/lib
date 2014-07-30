/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#if !defined(BUILDING_WXUTILS)
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#endif

#define WXUTILS_ATTACH_DATA_MAX_ARITY 10
#define WXUTILS_ATTACH_DATA_PRIVATE_CTR_DEF(z, n, data) \
    template <BOOST_PP_ENUM_PARAMS(n, class T)> \
    explicit AttachData(BOOST_PP_ENUM_BINARY_PARAMS(n, const T, & x)) \
        : Base(BOOST_PP_ENUM_PARAMS(n, x)) \
    { \
    } \
    /**/

namespace wxutils {

template<class Base, class Data>
class AttachData : public Base {
public:
    AttachData() {}

    BOOST_PP_REPEAT_FROM_TO(
        1, BOOST_PP_INC(WXUTILS_ATTACH_DATA_MAX_ARITY),
        WXUTILS_ATTACH_DATA_PRIVATE_CTR_DEF, _)

    const Data & attachedData() const
    {
        return data_;
    }

    Data & attachedData()
    {
        return data_;
    }

    void listen(const std::function<void()> & listener)
    {
        deathListener_ = listener;
    }

    ~AttachData()
    {
        if(deathListener_)
            deathListener_();
    }
private:
    Data data_;
    std::function<void()> deathListener_;
};

}
