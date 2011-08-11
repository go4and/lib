#pragma once

#ifndef NEXUS_BUILDING

#include <boost/aligned_storage.hpp>
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

template<size_t N, bool strict>
class HandlerStorage : HandlerStorageBase {
public:
    static const size_t buffer_size = N;

    HandlerStorage()
        : allocated_(false) {}

    ~HandlerStorage()
    {
        BOOST_ASSERT(!allocated_);
    }

    void * alloc(size_t size)
    {
        if(size <= buffer_size && !allocated_.cas(true, false))
            return storage_.address();

#ifndef NDEBUG
        if(size > buffer_size)
            allocationFailed(size, buffer_size);
#endif

        BOOST_ASSERT(!strict && size <= buffer_size);
        return ::operator new(size);
    }

    void free(void * data)
    {
        if(data == storage_.address())
        {
            BOOST_ASSERT(allocated_);
            allocated_ = false;
        } else
            ::operator delete(data);
    }
private:
    mstd::atomic<bool> allocated_;
    boost::aligned_storage<buffer_size> storage_;
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

const size_t receiveStorageSize = 0x200;
const size_t sendStorageSize    = 0x200;
const size_t waitStorageSize    = 0x200;
const size_t connectStorageSize = 0x200;
const size_t acceptStorageSize  = 0x200;
const size_t resolveStorageSize = 0x200;

#define NEXUS_DECLARE_HANDLER(suffix, cls, n, kind, strict) \
    BOOST_PP_EXPR_IF(n, template <BOOST_PP_ENUM_PARAMS(n, class T)>) \
    class Handle##suffix { \
    public: \
        explicit Handle##suffix(cls * t BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(n, const T, & m)) \
            : t_(t) BOOST_PP_ENUM_TRAILING(n, NEXUS_HANDLER_ASSIGN_ARGUMENT_DEF, ~) {} \
    \
        BOOST_PP_REPEAT_FROM_TO( \
            1, BOOST_PP_INC(NEXUS_HANDLER_MAX_ARITY), \
            NEXUS_HANDLER_FORWARD_DEF, (handle##suffix, n)) \
    private: \
        cls * t_; \
        BOOST_PP_REPEAT(n, NEXUS_HANDLER_MEMBER_DEF, ~) \
        \
        void * alloc(size_t size) \
        { \
            return t_->storage##suffix##_.alloc(size); \
        } \
        \
        friend void* asio_handler_allocate(size_t size, Handle##suffix BOOST_PP_EXPR_IF(n, <BOOST_PP_ENUM_PARAMS(n, T)>) * handler) \
        { \
            return handler->alloc(size); \
        } \
        \
        void free(void * data) \
        { \
            return t_->storage##suffix##_.free(data); \
        } \
        \
        friend void asio_handler_deallocate(void * data, size_t size, Handle##suffix BOOST_PP_EXPR_IF(n, <BOOST_PP_ENUM_PARAMS(n, T)>) * handler) \
        { \
            return handler->free(data); \
        } \
    }; \
    \
    BOOST_PP_EXPR_IF(n, template <BOOST_PP_ENUM_PARAMS(n, class T)>) \
    Handle##suffix BOOST_PP_EXPR_IF(n, <BOOST_PP_ENUM_PARAMS(n, T)>) bind##suffix (BOOST_PP_ENUM_BINARY_PARAMS(n, const T, & m)) \
    { \
        return Handle##suffix BOOST_PP_EXPR_IF(n, <BOOST_PP_ENUM_PARAMS(n, T)>)(this BOOST_PP_ENUM_TRAILING_PARAMS(n, m)); \
    } \
    \
    BOOST_PP_EXPR_IF(n, template <BOOST_PP_ENUM(n, NEXUS_GET_DATA, class)>) \
    friend class Handle##suffix; \
    nexus::HandlerStorage<nexus::kind##StorageSize, strict> storage##suffix##_; \
    /**/

#define NEXUS_DECLARE_SIMPLE_HANDLER(a, b, c) NEXUS_DECLARE_HANDLER(a, b, 0, c, true)

}
