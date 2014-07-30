/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#if !defined(BUILDING_CALC)
#include <boost/mpl/and.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/pop_front.hpp>

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/inc.hpp>
#include <boost/preprocessor/repeat.hpp>
#include <boost/preprocessor/repeat_from_to.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>

#include <boost/type_traits/is_pointer.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/function_traits.hpp>

#include <boost/utility/enable_if.hpp>
#endif

#define CALC_SIGN_MAX_ARITY 5
#define CALC_SIGN_PRIVATE_REDIRECT(z, n, data) \
    template <BOOST_PP_ENUM_PARAMS(n, typename T)> \
    typename boost::function_traits<S>::result_type operator()(void * context, BOOST_PP_ENUM_BINARY_PARAMS(n, T, x)) const \
    { \
        return f_(downcast<real_context>(context), BOOST_PP_ENUM_PARAMS(n, x)); \
    } \
    /**/

namespace calc {

template<class S, class F>
class sign_t : public boost::function_traits<S> {
public:
    template<class Args>
    struct sig {
        typedef typename boost::function_traits<S>::result_type type;
    };

    typedef void * arg1_type;
    typedef typename boost::function_traits<S>::arg1_type real_context;

    sign_t(const F & f) 
        : f_(f) {}

    typename boost::function_traits<S>::result_type operator()(void * context) const
    {
        return f_(downcast<real_context>(context));
    }

    BOOST_PP_REPEAT_FROM_TO(
        1, BOOST_PP_INC(CALC_SIGN_MAX_ARITY),
        CALC_SIGN_PRIVATE_REDIRECT, _ )
private:
    template<class T>
    static typename boost::enable_if<boost::is_same<T, void*>, T&>::type
    downcast(void *& context)
    {
        return context;
    }
    
    template<class T>
    static typename boost::enable_if<boost::mpl::and_<boost::is_pointer<T>, boost::mpl::not_<boost::is_same<T, void*> > >, T>::type
    downcast(void * context)
    {
        return static_cast<T>(context);
    }

    template<class T>
    static typename boost::enable_if<boost::is_reference<T>, T>::type
    downcast(void * context)
    {
        return *static_cast<typename boost::remove_reference<T>::type*>(context);
    }

    F f_;
};

template<class S, class F>
sign_t<S, F> sign(const F & f)
{
    return sign_t<S, F>(f);
}

#undef CALC_SIGN_PRIVATE_REDIRECT

#define CALC_DECLARE_SIGN_ARGUMENT(z, elem, data) typedef typename boost::mpl::at<Vector, boost::mpl::size_t<elem>>::type BOOST_PP_CAT(BOOST_PP_CAT(arg, BOOST_PP_INC(elem)), _type);

template<class Vector, size_t n>
struct named_arguments_helper;

#define CALC_NAMED_ARGUMENTS_HELPER(z, elem, data) \
    template<class Vector> \
    struct named_arguments_helper<Vector, elem> { \
        BOOST_PP_REPEAT(elem, CALC_DECLARE_SIGN_ARGUMENT, ~); \
    }; \
    /**/

BOOST_PP_REPEAT(13, CALC_NAMED_ARGUMENTS_HELPER, ~);

template<class Vector>
struct named_arguments : public named_arguments_helper<Vector, boost::mpl::size<Vector>::type::value> {
};

template<class Vector>
struct vector_function : public named_arguments<typename boost::mpl::pop_front<Vector>::type> {
    typedef Vector signature_vector;
    typedef typename boost::mpl::at<Vector, boost::mpl::size_t<0> >::type result_type;
    typedef typename boost::mpl::pop_front<Vector>::type arguments;
    static const size_t arity = boost::mpl::size<Vector>::type::value - 1;
};

template<class... Sign>
struct generic_function : public vector_function<boost::mpl::vector<Sign...> > {
};

#define CALC_DECLARE_SIGN(name, n) \
    template<BOOST_PP_ENUM_PARAMS(n, class Arg), class Result> \
    struct name : public generic_function<Result, BOOST_PP_ENUM_PARAMS(n, Arg) > { \
    }; \
    /**/

CALC_DECLARE_SIGN(unary_function, 1);
CALC_DECLARE_SIGN(binary_function, 2);
CALC_DECLARE_SIGN(ternary_function, 3);
CALC_DECLARE_SIGN(quaternary_function, 4);
CALC_DECLARE_SIGN(quinary_function, 5);
    
}
