#pragma once

#ifndef NEXUS_BUILDING
#include <type_traits>

#include <boost/preprocessor/cat.hpp>
#endif

namespace nexus {

namespace detail {

template<size_t ...> struct seq { };
template<size_t N, int ...S> struct genSeq : genSeq<N - 1, N - 1, S...> { };
template<size_t ...S> struct genSeq<0, S...> { typedef seq<S...> type; };

}

#define NEXUS_MEM_FUN_INVOKER_NAME(cls, function, binder) BOOST_PP_CAT(BOOST_PP_CAT(BOOST_PP_CAT(BOOST_PP_CAT(BOOST_PP_CAT(NEXUS_MEM_FUN_INVOKER_, cls), _), function), _), binder)

#define NEXUS_MEM_FUN_INVOKER(cls, function, binder) \
    template<class... Args> \
    class NEXUS_MEM_FUN_INVOKER_NAME(cls, function, binder) { \
    public: \
        explicit NEXUS_MEM_FUN_INVOKER_NAME(cls, function, binder)(cls * self, Args &&... args) \
            : self_(self), args_(std::forward<Args>(args)...) {} \
        NEXUS_MEM_FUN_INVOKER_NAME(cls, function, binder)(NEXUS_MEM_FUN_INVOKER_NAME(cls, function, binder) && rhs) \
            : self_(rhs.self_), args_(std::move(rhs.args_)) \
        { \
        } \
        void operator=(NEXUS_MEM_FUN_INVOKER_NAME(cls, function, binder) && rhs) \
        { \
            self_ = rhs.self_; \
            args_ = std::move(rhs.args_); \
        } \
        template<class... Params> \
        void operator()(Params &&... params) \
        { \
            invoke(typename nexus::detail::genSeq<sizeof...(Args)>::type(), std::forward<Params>(params)...); \
        } \
        cls * self() const { return self_; } \
    private: \
        template<size_t ...S, class... Params> \
        void invoke(nexus::detail::seq<S...>, Params &&... params) \
        { \
            self_->function(std::forward<Params>(params)..., std::get<S>(args_)...); \
        } \
        typedef std::tuple<typename std::decay<Args>::type...> arguments_type; \
        cls * self_; \
        arguments_type args_; \
    }; \
    template<class... Args> \
    NEXUS_MEM_FUN_INVOKER_NAME(cls, function, binder)<Args...> binder(Args &&... args) \
    { \
        return NEXUS_MEM_FUN_INVOKER_NAME(cls, function, binder)<Args...>(this, std::forward<Args>(args)...); \
    } \
    /**/

}
