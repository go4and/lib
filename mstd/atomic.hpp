#pragma once

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4146)
#endif

#include <cstddef>

#include <boost/cstdint.hpp>

#include <boost/mpl/if.hpp>
#include <boost/mpl/size_t.hpp>

#include <boost/static_assert.hpp>

#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_pointer.hpp>

#include <boost/utility/enable_if.hpp>

#include "detail/machine.hpp"

namespace mstd { namespace detail {

template<class Value>
struct minus_one {
    static const Value value = Value(0) - Value(1);
};

template<size_t Size>
struct size_to_word {
    typedef intptr_t type;
};

template<>
struct size_to_word<8> {
    typedef boost::int64_t type;
};

template<typename Value>
struct atomic_base {
    atomic_base() {}

    atomic_base(Value value)
        : value_(value) {}

    volatile Value value_;
};

#if __GNUC__ || __SUNPRO_CC
#define MSTD_FORCE_ALIGNMENT(decl, a) decl  __attribute__ ((aligned(a)));
#elif defined(__INTEL_COMPILER) || _MSC_VER >= 1300
#define MSTD_FORCE_ALIGNMENT(decl, a) __declspec(align(a)) decl;
#else 
#error Do not know syntax for forcing alignment.
#endif

template<>
struct atomic_base<boost::uint64_t> {
    atomic_base()
        : value_(0) {}

    atomic_base(boost::uint64_t value)
        : value_(value) {}

    MSTD_FORCE_ALIGNMENT(boost::uint64_t value_, 8)
};

template<>
struct atomic_base<boost::int64_t> {
    atomic_base()
        : value_(0) {}

    atomic_base(boost::int64_t value)
        : value_(value) {}

    MSTD_FORCE_ALIGNMENT(boost::int64_t value_, 8)
};

template<class T, bool integral>
struct atomic_size_helper;

template<class T>
struct atomic_size_helper<T, true> {
    typedef boost::mpl::size_t<1> type;
};

template<class T>
struct atomic_size_helper<T*, false> {
    typedef boost::mpl::size_t<sizeof(T)> type;
};

template<class Value>
class atomic_impl : private atomic_base<Value> {
public:
    typedef Value value_type;
    typedef typename boost::mpl::if_<
                boost::is_pointer<Value>,
                ptrdiff_t,
                Value
            >::type difference_type;

    atomic_impl() {}

    atomic_impl(value_type v)
        : atomic_base<Value>(v) {}

    value_type add(difference_type value)
    {
        typedef typename atomic_size_helper<value_type, boost::is_integral<value_type>::value>::type size;

        return value_type(helper::add(static_cast<volatile int_type*>(static_cast<volatile void*>(&this->value_)), value * size::value));
    }

    value_type increment() {
        return add(1);
    }

    value_type decrement()
    {
        return add(detail::minus_one<difference_type>::value);
    }

    value_type read_write(value_type value)
    {
        return value_type(helper::read_write(static_cast<volatile int_type*>(static_cast<volatile void*>(&this->value_)), word_type(value)));
    }

    value_type cas(value_type value, value_type cmp)
    {
        BOOST_STATIC_ASSERT(sizeof(int_type) == sizeof(this->value_));
        return value_type(helper::cas(static_cast<volatile int_type*>(static_cast<volatile void*>(&this->value_)), int_type(value), int_type(cmp)));
    }

    operator value_type() const volatile
    {
        return value_type(atomic_read<sizeof(value_type)>(&this->value_));
    }

    template<class F>
    value_type modify(F f)
    {
        return detail::generic_modify<sizeof(value_type)>(&this->value_, f);
    }

    value_type & _direct_reference() const
    {
        return const_cast<value_type&>(this->value_);
    }
protected:
    void atomic_write(value_type rhs)
    {
        detail::atomic_write<sizeof(value_type)>(&this->value_, int_type(rhs));
    }
public:
    value_type operator+=(difference_type value)
    {
        return add(value) + value;
    }

    value_type operator-=(difference_type value)
    {
        return operator+=(-value);    
    }

    value_type operator++()
    {
        return add(1) + 1;
    }

    value_type operator--()
    {
        return add(detail::minus_one<difference_type>::value) - 1;
    }

    value_type operator++(int)
    {
        return add(1);
    }

    value_type operator--(int)
    {
        return add(detail::minus_one<difference_type>::value);
    }
private:
    typedef typename size_to_word<sizeof(value_type)>::type word_type;
    typedef typename size_to_int<sizeof(value_type)>::type int_type;
    typedef typename detail::atomic_helper<sizeof(value_type)> helper;
};

}

template<typename T>
class atomic : public detail::atomic_impl<T> {
public:
    atomic() {}
    explicit atomic(T initial) : detail::atomic_impl<T>(initial) {}

    atomic<T> & operator=(T rhs)
    { 
        this->atomic_write(rhs);
        return *this;
    }

    atomic<T> & operator=(const atomic<T> & rhs)
    { 
        this->atomic_write(rhs);
        return *this;
    }

    T operator->() const
    {
        return pointee<T>();
    }
private:
    template<class U>
    typename boost::enable_if<boost::is_pointer<U>, T>::type
    pointee() const
    {
        return (*this);
    }
};

template<>
class atomic<bool> {
public:
    typedef bool value_type;

    atomic() {}
    explicit atomic(bool initial) : impl_(initial) {}

    bool cas(bool value, bool cmp)
    {
        return impl_.cas(value, cmp) != 0;
    }

    bool read_write(bool value)
    {
        return impl_.read_write(value) != 0;
    }

    operator bool() const
    {
        return impl_ != 0;
    }

    atomic<bool> & operator=(bool rhs)
    {
        impl_ = rhs;
        return *this;
    }

    atomic<bool> & operator=(const atomic<bool> & rhs)
    {
        impl_ = rhs;
        return *this;
    }
private:
    typedef detail::size_to_word<sizeof(bool)>::type word_type;
    atomic<word_type> impl_;
};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

}
