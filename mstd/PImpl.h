#pragma once

#include <boost/scoped_ptr.hpp>

#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>


#define MSTD_PIMPL_MAX_ARITY 5
#define MSTD_PIMPL_PRIVATE_CTR_DEF(z, n, data) \
    template <BOOST_PP_ENUM_PARAMS(n, typename T)> \
    explicit PImpl(BOOST_PP_ENUM_BINARY_PARAMS(n, T, & x)) \
    : impl_(new Impl(BOOST_PP_ENUM_PARAMS(n, x))) \
    { \
    } \
    /**/

namespace mstd {

template<class T>
class PImpl {
protected:
    PImpl() : impl_(new Impl) {}
    
    BOOST_PP_REPEAT_FROM_TO(
        1, BOOST_PP_INC(MSTD_PIMPL_MAX_ARITY),
        MSTD_PIMPL_PRIVATE_CTR_DEF, _ )

    ~PImpl() {}

    class Impl;
    
    const Impl & impl() const
    {
        return *impl_;
    }
    
    Impl & impl()
    {
        return *impl_;
    }

    void copyFrom(const Impl & src)
    {
        boost::scoped_ptr<Impl> newImpl(new Impl(src));
        impl_.swap(newImpl);
    }

    void swap(PImpl & rhs)
    {
        impl_.swap(rhs.impl_);
    }
private:    
    boost::scoped_ptr<Impl> impl_;
};

#undef MSTD_PIMPL_PRIVATE_CTR_DEF

}
