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
    typedef boost::tuples::null_type type;
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
class invoker {
public:
    typedef traits<F> traits_type;
    static const size_t arity = traits_type::arity;
    typedef boost::array<program, arity> args_type;

    explicit invoker(const F & f, const std::vector<program> & args)
        : f_(f)
    {
        BOOST_ASSERT(args.size() == arity);
        std::copy(args.begin(), args.end(), args_.begin());
    }
    
    variable operator()(void * context, variable * stack) const
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
                 convert<typename traits_type::template arg_type<0>::type>::apply(args_[0](context, stack)));
    }

    template<class U>
    typename boost::enable_if_c<traits<U>::arity == 2, variable>::type
    execute(const U & u, void * context, variable * stack) const
    {
        return u(context,
                 convert<typename traits_type::template arg_type<0>::type>::apply(args_[0](context, stack)),
                 convert<typename traits_type::template arg_type<1>::type>::apply(args_[1](context, stack)));
    }

    template<class U>
    typename boost::enable_if_c<traits<U>::arity == 3, variable>::type
    execute(const U & u, void * context, variable * stack) const
    {
        return u(context,
                 convert<typename traits_type::template arg_type<0>::type>::apply(args_[0](context, stack)),
                 convert<typename traits_type::template arg_type<1>::type>::apply(args_[1](context, stack)),
                 convert<typename traits_type::template arg_type<2>::type>::apply(args_[2](context, stack)));
    }

    template<class U>
    typename boost::enable_if_c<traits<U>::arity == 4, variable>::type
    execute(const U & u, void * context, variable * stack) const
    {
        return u(context,
                 convert<typename traits_type::template arg_type<0>::type>::apply(args_[0](context, stack)),
                 convert<typename traits_type::template arg_type<1>::type>::apply(args_[1](context, stack)),
                 convert<typename traits_type::template arg_type<2>::type>::apply(args_[2](context, stack)),
                 convert<typename traits_type::template arg_type<3>::type>::apply(args_[3](context, stack)));
    }

    template<class U>
    typename boost::enable_if_c<traits<U>::arity == 5, variable>::type
    execute(const U & u, void * context, variable * stack) const
    {
        return u(context,
                 convert<typename traits_type::template arg_type<0>::type>::apply(args_[0](context, stack)),
                 convert<typename traits_type::template arg_type<1>::type>::apply(args_[1](context, stack)),
                 convert<typename traits_type::template arg_type<2>::type>::apply(args_[2](context, stack)),
                 convert<typename traits_type::template arg_type<3>::type>::apply(args_[3](context, stack)),
                 convert<typename traits_type::template arg_type<4>::type>::apply(args_[4](context, stack)));
    }

    F f_;
    args_type args_;
};

template<class F>
class func_compiler {
public:
    func_compiler(const F & f) : f_(f) {}

    program operator()(const std::vector<program> & args, const function_lookup & lookup) const
    {
        return invoker<F>(f_, args);
    }
private:
    F f_;
};

}

}
