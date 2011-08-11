#pragma once

#include <vector>

#include <boost/mpl/if.hpp>

namespace mstd {

namespace detail {

template<class Key> struct key_traits;
template<> struct key_traits<boost::uint32_t> {
    typedef boost::uint16_t index_type;
    typedef boost::uint16_t version_type;
    static const int index_shift = 16;
    static const boost::uint16_t version_mask = 0xffff;
};

template<class Version, class Index, class Value>
struct tid_map_node {
    typedef Version version_type;
    typedef Value value_type;

    Value value;
    Version version;
    Index next;

    tid_map_node()
        : version(1) {}

    bool allocated() const
    {
        return !(version & 1);
    }
};

template<class Impl, bool isconst>
struct tid_map_iterator_value_type {
    typedef typename Impl::value_type node_type;
    typedef typename boost::remove_cv<node_type>::type::value_type raw_value_type;
    typedef typename boost::mpl::if_c<isconst, const raw_value_type, raw_value_type>::type value_type;
};

template<class Impl, bool isconst>
struct tid_map_iterator_get_base {
    typedef std::iterator<std::bidirectional_iterator_tag,
                          typename tid_map_iterator_value_type<Impl, isconst>::value_type> type;
};

template<class Impl, bool isconst>
class tid_map_iterator : public tid_map_iterator_get_base<Impl, isconst>::type {
public:
    typedef Impl impl_type;
    typedef typename tid_map_iterator_get_base<Impl, isconst>::type base_type;
    typedef tid_map_iterator<Impl, isconst> this_type;

    template<class I2>
    tid_map_iterator(const tid_map_iterator<I2, false> & rhs,
                     const typename boost::enable_if<
                        boost::mpl::and_<boost::is_convertible<I2, Impl>,
                                         boost::mpl::bool_<isconst> >,
                        int>::type dummy = 0)
        : impl_(rhs.impl()), end_(rhs.end()) {}

    tid_map_iterator(Impl impl, Impl end)
        : impl_(impl), end_(end) {}

    typename base_type::pointer operator->() const
    {
        return &impl_->value;
    }
    
    typename base_type::reference operator*() const
    {
        return impl_->value;
    }

    void fix()
    {
        while(impl_ != end_ && (!impl_->allocated()))
            ++impl_;
    }

    this_type operator++()
    {
        while(++impl_ != end_ && (!impl_->allocated()));
        return *this;
    }

    const impl_type & impl() const
    {
        return impl_;
    }

    const impl_type & end() const
    {
        return end_;
    }
private:
    Impl impl_;
    Impl end_;

    friend bool operator!=(const tid_map_iterator & lhs, const tid_map_iterator & rhs)
    {
        return lhs.impl_ != rhs.impl_;
    }

    friend bool operator==(const tid_map_iterator & lhs, const tid_map_iterator & rhs)
    {
        return lhs.impl_ == rhs.impl_;
    }
};

}

template<class Key>
class tid_map_key {
public:
    typedef detail::key_traits<Key> traits;
    typedef typename traits::version_type version_type;
    typedef typename traits::index_type index_type;

    tid_map_key(Key src)
        : impl_(src) {}

    tid_map_key(size_t index, version_type version)
        : impl_((index << traits::index_shift) | version)
    {
    }

    size_t index() const
    {
        return impl_ >> traits::index_shift;
    }

    version_type version() const
    {
        return impl_ & traits::version_mask;
    }

    Key full() const
    {
        return impl_;
    }
private:
    Key impl_;
};

template<class Key, class Value>
class tid_map {
public:
    typedef Key key_type;
    typedef Value mapped_type;
    typedef typename key_type::index_type index_type;
    typedef detail::tid_map_node<typename key_type::version_type, index_type, Value> node_type;
    typedef std::vector<node_type> nodes_type;
    typedef detail::tid_map_iterator<typename nodes_type::iterator, false> iterator;
    typedef detail::tid_map_iterator<typename nodes_type::const_iterator, true> const_iterator;

    tid_map()
        : head_(std::numeric_limits<index_type>::max()) {}

    iterator find(key_type key)
    {
        size_t idx = key.index();
        if(idx < nodes_.size())
        {
            typename nodes_type::iterator i = nodes_.begin() + idx;
            if(i->version == key.version())
                return iterator(i, nodes_.end());
        }
        return iterator(nodes_.end(), nodes_.end());
    }

    const_iterator find(key_type key) const
    {
        size_t idx = key.index();
        if(idx < nodes_.size())
        {
            typename nodes_type::const_iterator i = nodes_.begin() + idx;
            if(i->version == key.version())
                return const_iterator(i, nodes_.end());
        }
        return const_iterator(nodes_.end(), nodes_.end());
    }

    const_iterator begin() const
    {
        const_iterator result(nodes_.begin(), nodes_.end());
        result.fix();
        return result;
    }

    const_iterator end() const
    {
        return const_iterator(nodes_.end(), nodes_.end());
    }

    size_t erase(key_type key)
    {
        iterator i = find(key);
        if(i.impl() != nodes_.end())
        {
            erase(i);
            return 1;
        } else
            return 0;
    }

    void erase(iterator i)
    {
        typename iterator::impl_type impl = i.impl();
        ++impl->version;
        impl->next = head_;
        impl->value = 0;
        head_ = impl - nodes_.begin();
    }

    key_type insert(const mapped_type & value)
    {
        size_t index;
        node_type * node;
        if(head_ == std::numeric_limits<index_type>::max())
        {
            index = nodes_.size();
            nodes_.resize(index + 1);
            node = &nodes_[index];
            node->version = 0;
        } else {
            index = head_;
            node = &nodes_[index];
            head_ = node->next;
            ++node->version;
        }

        node->value = value;

        return key_type(index, nodes_[index].version);
    }

    size_t count_size() const
    {
        return std::distance(begin(), end());
    }
private:
    nodes_type nodes_;
    index_type head_;
};

}
