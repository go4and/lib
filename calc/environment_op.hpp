/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#include "conversion.hpp"

namespace calc {

namespace detail {

template<class F>
struct traits_helper : boost::mpl::if_<boost::is_function<typename boost::remove_pointer<F>::type>, 
                                       boost::function_traits<typename boost::remove_pointer<F>::type>,
                                       F>::type {};

template<class F, bool A, int N>
struct arg_type;

template<class F, int N>
struct arg_type<F, false, N> {
};

#define CALC_ARG_TYPE(z, index, data) \
template<class F> \
struct arg_type<F, true, index> { \
     typedef typename traits_helper<F>::BOOST_PP_CAT(arg, BOOST_PP_CAT(index, _type)) type; \
};

BOOST_PP_REPEAT_FROM_TO(1, 13, CALC_ARG_TYPE, ~);

template<class F>
struct traits {
    typedef traits_helper<F> helper;
    BOOST_STATIC_CONSTANT(int, arity = helper::arity - 1);
    typedef typename helper::arg1_type context_argument;

    template<int N>
    struct arg_type {
        typedef typename boost::remove_const<typename boost::remove_reference<typename detail::arg_type<F, N < arity, N + 2>::type>::type>::type type;
    };
};

template<class F>
class invoker : public pre_program {
public:
    typedef traits<F> traits_type;
    static const size_t arity = traits_type::arity;
    typedef boost::array<pre_program_ptr, arity> args_type;

    explicit invoker(const F & f, std::vector<pre_program*> & args)
        : f_(f)
    {
        BOOST_ASSERT(args.size() == arity);
        for(size_t i = 0; i != args_.size(); ++i)
        {
            args_[i].reset(args[i]);
            args[i] = 0;
        }
    }
    
    variable run(void * context, variable * stack) const
    {
        return execute(f_, context, stack);
    }
private:
    template<class U>
    typename boost::enable_if_c<traits<U>::arity == 0, variable>::type
    execute(const U & u, void * context, variable * stack) const
    {
        return u(context);
    }

#define CALC_INVOKER_EXECUTE_ARG(z, index, data) \
    convert<typename traits_type::template arg_type<index>::type>::apply(args_[index]->run(context, stack))
#define CALC_INVOKER_EXECUTE(z, index, data) \
    template<class U> \
    typename boost::enable_if_c<traits<U>::arity == index, variable>::type \
    execute(const U & u, void * context, variable * stack) const \
    { \
        return u(context, BOOST_PP_ENUM(index, CALC_INVOKER_EXECUTE_ARG, ~)); \
    }

    BOOST_PP_REPEAT_FROM_TO(1, 12, CALC_INVOKER_EXECUTE, ~);

    F f_;
    args_type args_;
};

template<class F>
class func_compiler {
public:
    func_compiler(const F & f) : f_(f) {}

    pre_program * operator()(std::vector<pre_program*> & args, const compiler_context & context) const
    {
        return new invoker<F>(f_, args);
    }
private:
    F f_;
};

}

}
