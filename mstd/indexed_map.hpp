/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

namespace mstd {

template<class Key, class Value, class Context>
class indexed_map {
public:
    typedef Key key_type;
    typedef Value mapped_type;
    typedef Value & reference;
    typedef const Value & const_reference;

    explicit indexed_map(const Context & context)
        : context_(context), impl_(context_.size())
    {
    }

    void clear(const mapped_type & value = mapped_type())
    {
        std::fill(impl_.begin(), impl_.end(), value);
    }

    reference operator[](const Key & key)
    {
        return impl_[context_.index(key)];
    }

    const_reference operator[](const Key & key) const
    {
        return impl_[context_.index(key)];
    }
private:
    typedef std::vector<mapped_type> Impl;
    Context context_;
    Impl impl_;
};

}
