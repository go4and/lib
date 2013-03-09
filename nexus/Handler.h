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

template<bool strict, size_t count>
class HandlerStorage : HandlerStorageBase {
public:
    HandlerStorage()
    {
    }

    void * alloc(size_t size)
    {
        for(size_t i = 0; i != count; ++i)
        {
            void * result = items_[i].alloc(size);
            if(result)
                return result;
        }

        BOOST_ASSERT(!strict);
        return malloc(size);
    }

    void free(void * data)
    {
        for(size_t i = 0; i != count; ++i)
            if(items_[i].free(data))
                return;
        ::free(data);
    }
private:
    struct StorageItem {
        mstd::atomic<bool> allocated;
        size_t storageSize;
        void * storage;
        
        StorageItem()
            : allocated(false), storageSize(0), storage(0)
        {
        }

        ~StorageItem()
        {
            BOOST_ASSERT(!allocated);
            if(storage)
                ::free(storage);
        }

        void * alloc(size_t size)
        {
            if(!allocated.cas(true, false))
            {
                if(size <= storageSize)
                    return storage;
                else {
                    if(storage)
                        ::free(storage);
                    storage = malloc(size);
                    storageSize = size;
                    return storage;
                }
            }
            return 0;
        }

        bool free(void * data)
        {
            if(data == storage)
            {
                BOOST_ASSERT(allocated);
                allocated = false;
                return true;
            } else
                return false;
        }
    };
    
    boost::array<StorageItem, count> items_;
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

#define NEXUS_HANDLER_MAKE_TEMPLATE_ZERO(n)
#define NEXUS_HANDLER_MAKE_TEMPLATE_IMPL(n) template <BOOST_PP_ENUM_PARAMS(n, class T)>
#define NEXUS_HANDLER_MAKE_TEMPLATE(n) BOOST_PP_IF(BOOST_PP_BOOL(n), NEXUS_HANDLER_MAKE_TEMPLATE_IMPL, NEXUS_HANDLER_MAKE_TEMPLATE_ZERO)(n)

#define NEXUS_HANDLER_MAKE_TEMPLATE_ARGS(n) <BOOST_PP_ENUM_PARAMS(n, T)>

#define NEXUS_HANDLER_FORWARD(n, suffix) \
        BOOST_PP_REPEAT_FROM_TO( \
            1, BOOST_PP_INC(NEXUS_HANDLER_MAX_ARITY), \
            NEXUS_HANDLER_FORWARD_DEF, (BOOST_PP_CAT(handle, suffix) , n))

#define NEXUS_HANDLER_FORWARD_ZERO(n, suffix) \
    void operator()() \
    { \
        t_->BOOST_PP_CAT(handle, suffix)(BOOST_PP_ENUM(n, NEXUS_HANDLER_PASS_MEMBER_DEF, ~)); \
    } \
    /**/


#define NEXUS_HANDLER_DECLARE_CLASS_IMPL(suffix, cls, n, name, templ, forward) \
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
        forward(n, suffix) \
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
    }; \
    /**/

#define NEXUS_HANDLER_CLASS(suffix, cls, n) \
    NEXUS_HANDLER_DECLARE_CLASS_IMPL(suffix, cls, n, BOOST_PP_CAT(BOOST_PP_CAT(Handle, suffix), n), NEXUS_HANDLER_MAKE_TEMPLATE(n), NEXUS_HANDLER_FORWARD)

#define NEXUS_HANDLER_ZERO_CLASS(suffix, cls, n) \
    NEXUS_HANDLER_DECLARE_CLASS_IMPL(suffix, cls, n, BOOST_PP_CAT(BOOST_PP_CAT(Handle, suffix), n), NEXUS_HANDLER_MAKE_TEMPLATE(n), NEXUS_HANDLER_FORWARD_ZERO)

#define NEXUS_HANDLER_BIND_DEF_IMPL(suffix, cls, n, classdef) \
    classdef(suffix, cls, n) \
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
    NEXUS_HANDLER_BIND_DEF_IMPL(BOOST_PP_TUPLE_ELEM(3, 0, data), BOOST_PP_TUPLE_ELEM(3, 1, data), n, BOOST_PP_TUPLE_ELEM(3, 2, data))

#define NEXUS_HANDLER_BIND_ZERO(suffix) \
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
    /**/

#define NEXUS_DECLARE_HANDLER_EX(suffix, cls, strict, count) \
    NEXUS_HANDLER_CLASS(suffix, cls, 0) \
    NEXUS_HANDLER_BIND_ZERO(suffix) \
    BOOST_PP_REPEAT_FROM_TO(1, BOOST_PP_INC(6), NEXUS_HANDLER_BIND_DEF, (suffix, cls, NEXUS_HANDLER_CLASS)) \
    nexus::HandlerStorage<strict, count> storage##suffix##_; \
    /**/

#define NEXUS_DECLARE_HANDLER(suffix, cls, strict) NEXUS_DECLARE_HANDLER_EX(suffix, cls, strict, 1)
#define NEXUS_DECLARE_SIMPLE_HANDLER(a, b) NEXUS_DECLARE_HANDLER(a, b, true)

#define NEXUS_HANDLER_ZERO(suffix, cls, count) \
    BOOST_PP_REPEAT_FROM_TO(1, BOOST_PP_INC(6), NEXUS_HANDLER_BIND_DEF, (suffix, cls, NEXUS_HANDLER_ZERO_CLASS)) \
    nexus::HandlerStorage<(count > 0), (count > 0 ? count : -count)> storage##suffix##_; \
    /**/

}
