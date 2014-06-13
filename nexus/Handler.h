/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#ifndef NEXUS_BUILDING

#include <boost/array.hpp>
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

#include "MemFunInvoker.h"

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

#define NEXUS_DECLARE_HANDLER_EX(suffix, cls, strict, count) \
    NEXUS_MEM_FUN_INVOKER(cls, BOOST_PP_CAT(handle, suffix), BOOST_PP_CAT(bind, suffix)); \
    template<class... Args> \
    friend void* asio_handler_allocate(size_t size, NEXUS_MEM_FUN_INVOKER_NAME(cls, BOOST_PP_CAT(handle, suffix), BOOST_PP_CAT(bind, suffix))<Args...> * handler) \
    { \
        return handler->self()->BOOST_PP_CAT(BOOST_PP_CAT(storage, suffix), _).alloc(size); \
    } \
    \
    template<class... Args> \
    friend void asio_handler_deallocate(void * data, size_t size, NEXUS_MEM_FUN_INVOKER_NAME(cls, BOOST_PP_CAT(handle, suffix), BOOST_PP_CAT(bind, suffix))<Args...> * handler) \
    { \
        return handler->self()->BOOST_PP_CAT(BOOST_PP_CAT(storage, suffix), _).free(data); \
    } \
    nexus::HandlerStorage<strict, count> BOOST_PP_CAT(BOOST_PP_CAT(storage, suffix), _); \
    /**/

#define NEXUS_DECLARE_HANDLER(suffix, cls, strict) NEXUS_DECLARE_HANDLER_EX(suffix, cls, strict, 1)
#define NEXUS_DECLARE_SIMPLE_HANDLER(a, b) NEXUS_DECLARE_HANDLER(a, b, true)
#define NEXUS_HANDLER(suffix, cls, count) NEXUS_DECLARE_HANDLER_EX(suffix, cls, (count > 0), (count > 0 ? count : -count))

}
