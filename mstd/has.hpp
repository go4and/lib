#pragma once

#include <boost/preprocessor/cat.hpp>

#include <boost/mpl/bool.hpp>

#define MSTD_HAS_STATIC_FUNCTION_TRAIT_DEF(name, sign, fname) \
    template<class T> \
    struct BOOST_PP_CAT(name, _helper) { \
        struct yes { char a; }; \
        struct no { yes b[2]; }; \
        template<typename U, sign> struct helper {}; \
        template<typename U> static yes test(helper<U, &U::fname>*); \
        template<typename U> static no test(...); \
        static const bool value = sizeof(test<T>(0)) == sizeof(yes); \
    }; \
    \
    template<class T> \
    struct name : public boost::mpl::bool_<BOOST_PP_CAT(name, _helper)<T>::value> { \
    }; \
    /**/
