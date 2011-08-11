#pragma once

#include <boost/static_assert.hpp>

namespace mstd {

template<class Tag, size_t Id>
class enum_set_item {
public:
    BOOST_STATIC_ASSERT(Id < Tag::value);
    static const size_t id = Id;
    
    enum_set_item() {}

    static const enum_set_item & get()
    {
        static const enum_set_item result;
        return result;
    }
};

template<class Tag>
class enum_set {
private:
    struct dummy {
      void nonnull() {};
    };

    typedef void (dummy::*safe_bool)();
public:
    typedef size_t value_type;

    enum_set() : value_(0) {}

    template<size_t Id>
    enum_set(enum_set_item<Tag, Id>)
        : value_(1 << Id) {}
        
    explicit enum_set(const value_type & mask)
        : value_(mask)
    {
    }

    template<class Item>
    bool has() const
    {
        return has(Item::get());
    }
    
    template<size_t Id>
    bool has(enum_set_item<Tag, Id>) const
    {
        return (value_ & (1 << Id)) != 0;
    }
        
    value_type & value()
    {
        return value_;
    }

    const value_type & value() const
    {
        return value_;
    }
    
    operator safe_bool() const
    {
        return value_ != 0 ? &dummy::nonnull : 0;
    }

    static enum_set any()
    {
        return enum_set(static_cast<value_type>((1 << Tag::value) - 1));
    }
private:
    value_type value_;
};

template<class Tag, size_t Id>
enum_set<Tag> & operator|=(enum_set<Tag> & lhs, enum_set_item<Tag, Id> rhs)
{
    lhs.value() |= enum_set<Tag>(rhs).value();
    return lhs;
}

template<class Tag, size_t Id>
enum_set<Tag> & operator^=(enum_set<Tag> & lhs, enum_set_item<Tag, Id> rhs)
{
    lhs.value() ^= enum_set<Tag>(rhs).value();
    return lhs;
}

template<class Tag>
enum_set<Tag> & operator&=(enum_set<Tag> & lhs, enum_set<Tag> rhs)
{
    lhs.value() &= rhs.value();
    return lhs;
}

template<class Tag, size_t Id>
enum_set<Tag> & operator&=(enum_set<Tag> & lhs, enum_set_item<Tag, Id> rhs)
{
    lhs.value() &= enum_set<Tag>(rhs).value();
    return lhs;
}

template<class Tag>
enum_set<Tag> operator&(const enum_set<Tag> & lhs, const enum_set<Tag> & rhs)
{
    return enum_set<Tag>(lhs.value() & rhs.value());
}

template<class Tag, size_t Id>
enum_set<Tag> operator&(const enum_set<Tag> & lhs, enum_set_item<Tag, Id> rhs)
{
    return lhs & enum_set<Tag>(rhs);
}

template<class Tag, size_t Id1, size_t Id2>
enum_set<Tag> operator|(enum_set_item<Tag, Id1> lhs, enum_set_item<Tag, Id2> rhs)
{
    return enum_set<Tag>(lhs) | enum_set<Tag>(rhs);
}

template<class Tag, size_t Id>
enum_set<Tag> operator|(const enum_set<Tag> & lhs, enum_set_item<Tag, Id> rhs)
{
    return lhs | enum_set<Tag>(rhs);
}

template<class Tag>
enum_set<Tag> operator|(const enum_set<Tag> & lhs, const enum_set<Tag> & rhs)
{
    return enum_set<Tag>(lhs.value() | rhs.value());
}

template<class Tag>
enum_set<Tag> operator~(const enum_set<Tag> & src)
{
    return enum_set<Tag>(~src.value());
}

template<class Tag, size_t Id>
enum_set<Tag> operator~(enum_set_item<Tag, Id> src)
{
    return ~enum_set<Tag>(src);
}

template<class Tag>
bool operator<(const enum_set<Tag> & lhs, const enum_set<Tag> & rhs)
{
    return lhs.value() < rhs.value();
}

}
