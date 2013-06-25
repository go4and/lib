/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#namespace calc {

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

template<class Arg1, class Result>
struct unary_function {
    typedef Result result_type;
    typedef Arg1 arg1_type;
    BOOST_STATIC_CONSTANT(size_t, arity = 1);
};

template<class Arg1, class Arg2, class Result>
struct binary_function {
    typedef Result result_type;
    typedef Arg1 arg1_type;
    typedef Arg2 arg2_type;
    BOOST_STATIC_CONSTANT(size_t, arity = 2);
};

template<class Arg1, class Arg2, class Arg3, class Result>
struct ternary_function {
    typedef Result result_type;
    typedef Arg1 arg1_type;
    typedef Arg2 arg2_type;
    typedef Arg3 arg3_type;
    BOOST_STATIC_CONSTANT(size_t, arity = 3);
};

template<class Arg1, class Arg2, class Arg3, class Arg4, class Result>
struct quaternary_function {
    typedef Result result_type;
    typedef Arg1 arg1_type;
    typedef Arg2 arg2_type;
    typedef Arg3 arg3_type;
    typedef Arg4 arg4_type;
    BOOST_STATIC_CONSTANT(size_t, arity = 4);
};

template<class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Result>
struct quinary_function {
    typedef Result result_type;
    typedef Arg1 arg1_type;
    typedef Arg2 arg2_type;
    typedef Arg3 arg3_type;
    typedef Arg4 arg4_type;
    typedef Arg5 arg5_type;
    BOOST_STATIC_CONSTANT(size_t, arity = 5);
};

}
