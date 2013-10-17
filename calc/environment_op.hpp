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

template<class F>
struct arg_type<F, true, 1> {
    typedef typename traits_helper<F>::arg1_type type;
};

template<class F>
struct arg_type<F, true, 2> {
    typedef typename traits_helper<F>::arg2_type type;
};

template<class F>
struct arg_type<F, true, 3> {
    typedef typename traits_helper<F>::arg3_type type;
};

template<class F>
struct arg_type<F, true, 4> {
    typedef typename traits_helper<F>::arg4_type type;
};

template<class F>
struct arg_type<F, true, 5> {
    typedef typename traits_helper<F>::arg5_type type;
};

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

    template<class U>
    typename boost::enable_if_c<traits<U>::arity == 1, variable>::type
    execute(const U & u, void * context, variable * stack) const
    {
        return u(context,
                 convert<typename traits_type::template arg_type<0>::type>::apply(args_[0]->run(context, stack)));
    }

    template<class U>
    typename boost::enable_if_c<traits<U>::arity == 2, variable>::type
    execute(const U & u, void * context, variable * stack) const
    {
        return u(context,
                 convert<typename traits_type::template arg_type<0>::type>::apply(args_[0]->run(context, stack)),
                 convert<typename traits_type::template arg_type<1>::type>::apply(args_[1]->run(context, stack)));
    }

    template<class U>
    typename boost::enable_if_c<traits<U>::arity == 3, variable>::type
    execute(const U & u, void * context, variable * stack) const
    {
        return u(context,
                 convert<typename traits_type::template arg_type<0>::type>::apply(args_[0]->run(context, stack)),
                 convert<typename traits_type::template arg_type<1>::type>::apply(args_[1]->run(context, stack)),
                 convert<typename traits_type::template arg_type<2>::type>::apply(args_[2]->run(context, stack)));
    }

    template<class U>
    typename boost::enable_if_c<traits<U>::arity == 4, variable>::type
    execute(const U & u, void * context, variable * stack) const
    {
        return u(context,
                 convert<typename traits_type::template arg_type<0>::type>::apply(args_[0]->run(context, stack)),
                 convert<typename traits_type::template arg_type<1>::type>::apply(args_[1]->run(context, stack)),
                 convert<typename traits_type::template arg_type<2>::type>::apply(args_[2]->run(context, stack)),
                 convert<typename traits_type::template arg_type<3>::type>::apply(args_[3]->run(context, stack)));
    }

    template<class U>
    typename boost::enable_if_c<traits<U>::arity == 5, variable>::type
    execute(const U & u, void * context, variable * stack) const
    {
        return u(context,
                 convert<typename traits_type::template arg_type<0>::type>::apply(args_[0]->run(context, stack)),
                 convert<typename traits_type::template arg_type<1>::type>::apply(args_[1]->run(context, stack)),
                 convert<typename traits_type::template arg_type<2>::type>::apply(args_[2]->run(context, stack)),
                 convert<typename traits_type::template arg_type<3>::type>::apply(args_[3]->run(context, stack)),
                 convert<typename traits_type::template arg_type<4>::type>::apply(args_[4]->run(context, stack)));
    }

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
