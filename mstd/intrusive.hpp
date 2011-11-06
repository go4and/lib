#pragma once

#include <boost/intrusive/list.hpp>
#include <boost/intrusive/set.hpp>

#include "disposers.hpp"

namespace mstd {

struct new_cloner {
    template<class T>
    T * operator()(const T & t) const
    {
        return new T(t);
    }
};

template<class Key>
struct key_compare {
    template<class T>
    bool operator()(const T & lhs, const Key & rhs) const
    {
        return lhs.key < rhs;
    }

    template<class T>
    bool operator()(const Key & lhs, const T & rhs) const
    {
        return lhs < rhs.key;
    }
};

struct identity {
    template<class T>
    T operator()(const T & t) const
    {
        return t;
    }
};

template<class T, class Key, Key T::*member>
struct key_equals {
    bool operator()(const T & lhs, const Key & rhs) const
    {
        return lhs.*member == rhs;
    }
    
    bool operator()(const Key & lhs, const T & rhs) const
    {
        return lhs == rhs.*member;
    }
    
    bool operator()(const T & lhs, const T & rhs) const
    {
        return lhs.*member == rhs.*member;
    }
};

template<class T>
class cloned_impl {
public:
    explicit cloned_impl(const T & t)
    {
        t_.clone_from(t, mstd::new_cloner(), mstd::delete_disposer());
    }

    cloned_impl(const cloned_impl & rhs)
    {
        t_.clone_from(rhs.t_, mstd::new_cloner(), mstd::delete_disposer());
    }

    ~cloned_impl()
    {
        t_.clear_and_dispose(mstd::delete_disposer());
    }

    operator T&()
    {
        return t_;
    }
private:
    T t_;
};

template<class T>
cloned_impl<T> cloned(const T & t)
{
    return cloned_impl<T>(t);
}

template<class T>
struct make_intrusive_list {
    typedef boost::intrusive::list<
                T,
                boost::intrusive::member_hook<T, boost::intrusive::list_member_hook<>, &T::listHook>
            > type;
};

template<class Extractor, class Comparator = std::less<typename Extractor::result_type> >
class extractor_compare {
public:
    typedef typename Extractor::result_type key_type;
    
    extractor_compare(const Extractor & extractor = Extractor(), const Comparator & comparator = Comparator())
        : extractor_(extractor), comparator_(comparator) {}
    
    template<class T>
    bool operator()(const T & lhs, const T & rhs) const
    {
        return comparator_(extractor_(lhs), extractor_(rhs));
    }
    
    template<class T>
    bool operator()(const key_type & lhs, const T & rhs) const
    {
        return comparator_(lhs, extractor_(rhs));
    }
    
    template<class T>
    bool operator()(const T & lhs, const key_type & rhs) const
    {
        return comparator_(extractor_(lhs), rhs);
    }

    template<class T>
    bool operator()(const key_type & lhs, const key_type & rhs) const
    {
        return comparator_(lhs, rhs);
    }
private:
    Extractor extractor_;
    Comparator comparator_;
};

template<class T, class Extractor, boost::intrusive::set_member_hook<> T::*hook = &T::setHook>
struct make_intrusive_set {
    typedef boost::intrusive::set<
                T,
                boost::intrusive::member_hook<T, boost::intrusive::set_member_hook<>, hook>,
                boost::intrusive::compare<extractor_compare<Extractor> >
            > type;
};

}
