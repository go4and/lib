#pragma once

#ifndef NEXUS_BUILDING

#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>

#include <boost/preprocessor/expand.hpp>
#include <boost/preprocessor/expr_if.hpp>
#include <boost/preprocessor/if.hpp>

#include <boost/preprocessor/repetition/enum.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing.hpp>
#include <boost/preprocessor/repetition/enum_trailing_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>

#include <mstd/atomic.hpp>

#endif

namespace nexus {

class HandlerStorageBase  : public boost::noncopyable {
protected:
    void allocationFailed(size_t size, size_t bufferSize);
};

template<bool strict>
class HandlerStorage : HandlerStorageBase {
public:
    HandlerStorage()
        : allocated_(false), storageSize_(0), storage_(0) {}

    ~HandlerStorage()
    {
        BOOST_ASSERT(!allocated_);
        if(storage_)
            ::free(storage_);
    }

    void * alloc(size_t size)
    {
        if(!allocated_.cas(true, false))
        {
            if(size <= storageSize_)
                return storage_;
            else {
                if(storage_)
                    ::free(storage_);
                storage_ = malloc(size);
                storageSize_ = size;
                return storage_;
            }
        }

        BOOST_ASSERT(!strict);
        return malloc(size);
    }

    void free(void * data)
    {
        if(data == storage_)
        {
            BOOST_ASSERT(allocated_);
            allocated_ = false;
        } else
            ::free(data);
    }
private:
    mstd::atomic<bool> allocated_;
    size_t storageSize_;
    void * storage_;
};

#define NEXUS_GET_FIRST(a, b) a
#define NEXUS_GET_SECOND(a, b) b
#define NEXUS_GET_DATA(z, n, data) data
#define NEXUS_HANDLER_PASS_MEMBER_DEF(z, n, data) BOOST_PP_CAT(BOOST_PP_CAT(m, n), _)

#define NEXUS_HANDLER_MAX_ARITY 3
#define NEXUS_HANDLER_FORWARD_DEF(z, n, data) \
    template <BOOST_PP_ENUM_PARAMS(n, class A)> \
    void operator()(BOOST_PP_ENUM_BINARY_PARAMS(n, const A, & a)) \
    { \
        t_->NEXUS_GET_FIRST data (BOOST_PP_ENUM_PARAMS(n, a) \
                                  BOOST_PP_ENUM_TRAILING(NEXUS_GET_SECOND data, NEXUS_HANDLER_PASS_MEMBER_DEF, ~)); \
    } \
    /**/

#define NEXUS_HANDLER_ASSIGN_ARGUMENT_DEF(z, n, data) BOOST_PP_CAT(BOOST_PP_CAT(m, n), _)(BOOST_PP_CAT(m, n))
#define NEXUS_HANDLER_MEMBER_DEF(z, n, data) BOOST_PP_CAT(T, n) BOOST_PP_CAT(BOOST_PP_CAT(m, n), _);

#define NEXUS_HANDLER_MAKE_EMPTY(n)
#define NEXUS_HANDLER_MAKE_TEMPLATE_ARGS(n) <BOOST_PP_ENUM_PARAMS(n, T)>
#define NEXUS_HANDLER_MAKE_TEMPLATE(n) template <BOOST_PP_ENUM_PARAMS(n, class T)>

#define NEXUS_DECLARE_HANDLER_CLASS_IMPL(suffix, cls, n, name, templ) \
    templ \
    class name; \
    \
    templ \
    friend class name; \
    \
    templ \
    class name { \
    public: \
        explicit name(cls * t BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(n, const T, & m)) \
            : t_(t) BOOST_PP_ENUM_TRAILING(n, NEXUS_HANDLER_ASSIGN_ARGUMENT_DEF, ~) {} \
        \
        BOOST_PP_REPEAT_FROM_TO( \
            1, BOOST_PP_INC(NEXUS_HANDLER_MAX_ARITY), \
            NEXUS_HANDLER_FORWARD_DEF, (BOOST_PP_CAT(handle, suffix) , n)) \
        \
        void * alloc(size_t size) \
        { \
            return t_->storage##suffix##_.alloc(size); \
        } \
        \
        void free(void * data) \
        { \
            return t_->storage##suffix##_.free(data); \
        } \
    private: \
        cls * t_; \
        BOOST_PP_REPEAT(n, NEXUS_HANDLER_MEMBER_DEF, ~) \
        \
    };

#define NEXUS_DECLARE_HANDLER_CLASS(suffix, cls, n) \
    NEXUS_DECLARE_HANDLER_CLASS_IMPL(suffix, cls, n, BOOST_PP_CAT(BOOST_PP_CAT(Handle, suffix), n), \
                                     BOOST_PP_IF(BOOST_PP_BOOL(n), NEXUS_HANDLER_MAKE_TEMPLATE, NEXUS_HANDLER_MAKE_EMPTY)(n))

#define NEXUS_HANDLER_BIND_DEF_IMPL(suffix, cls, n) \
    NEXUS_DECLARE_HANDLER_CLASS(suffix, cls, n) \
    \
    NEXUS_HANDLER_MAKE_TEMPLATE(n) \
    BOOST_PP_CAT(BOOST_PP_CAT(Handle, suffix), n) NEXUS_HANDLER_MAKE_TEMPLATE_ARGS(n) BOOST_PP_CAT(bind, suffix) (BOOST_PP_ENUM_BINARY_PARAMS(n, const T, & m)) \
    { \
        return BOOST_PP_CAT(BOOST_PP_CAT(Handle, suffix), n) NEXUS_HANDLER_MAKE_TEMPLATE_ARGS(n)(this BOOST_PP_ENUM_TRAILING_PARAMS(n, m)); \
    } \
    \
    NEXUS_HANDLER_MAKE_TEMPLATE(n) \
    friend void* asio_handler_allocate(size_t size, BOOST_PP_CAT(BOOST_PP_CAT(Handle, suffix), n) NEXUS_HANDLER_MAKE_TEMPLATE_ARGS(n) * handler) \
    { \
        return handler->alloc(size); \
    } \
    \
    NEXUS_HANDLER_MAKE_TEMPLATE(n) \
    friend void asio_handler_deallocate(void * data, size_t size, BOOST_PP_CAT(BOOST_PP_CAT(Handle, suffix), n) NEXUS_HANDLER_MAKE_TEMPLATE_ARGS(n) * handler) \
    { \
        return handler->free(data); \
    } \
    /**/

#define NEXUS_HANDLER_BIND_DEF(z, n, data) \
    NEXUS_HANDLER_BIND_DEF_IMPL(BOOST_PP_TUPLE_ELEM(2, 0, data), BOOST_PP_TUPLE_ELEM(2, 1, data), n)

#define NEXUS_DECLARE_HANDLER(suffix, cls, strict) \
    NEXUS_DECLARE_HANDLER_CLASS(suffix, cls, 0) \
    BOOST_PP_CAT(BOOST_PP_CAT(Handle, suffix), 0) BOOST_PP_CAT(bind, suffix)() \
    { \
        return BOOST_PP_CAT(BOOST_PP_CAT(Handle, suffix), 0)(this); \
    } \
    \
    friend void* asio_handler_allocate(size_t size, BOOST_PP_CAT(BOOST_PP_CAT(Handle, suffix), 0) * handler) \
    { \
        return handler->alloc(size); \
    } \
    \
    friend void asio_handler_deallocate(void * data, size_t size, BOOST_PP_CAT(BOOST_PP_CAT(Handle, suffix), 0) * handler) \
    { \
        return handler->free(data); \
    } \
    \
    BOOST_PP_REPEAT_FROM_TO(1, BOOST_PP_INC(6), NEXUS_HANDLER_BIND_DEF, (suffix, cls)) \
    nexus::HandlerStorage<strict> storage##suffix##_; \
    /**/

#define NEXUS_DECLARE_SIMPLE_HANDLER(a, b) NEXUS_DECLARE_HANDLER(a, b, true)

}
