/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#include <string>
#include <vector>

namespace mstd {

template<class Char, class Container> class matcher;

// TODO: sdelat' po papski kak u Alexandresku
template<class Char=wchar_t, class Container=std::basic_string<Char> >
class pattern {
public:
    template<class Cont>
    explicit pattern(const Cont & cont) : container_(cont.begin(), cont.end()) 
    {
        init();
    }
    
    template<class Iterator>
    pattern(Iterator begin, Iterator end) : container_(begin, end) 
    {
        init();
    }
    
    typename Container::size_type size() const
    {
        return container_.size();
    }

    int feed(Char ch, int len) const
    {
        while(len != -1 && ch != container_[len])
            len = p_[len];
        return ++len;
    }
private:
    void init()
    {
        if(!container_.empty())
        {
            int k = -1;
            p_.push_back(k);
            for(size_t j = 0, m = container_.size() - 1; j < m; ++j)
            {
                Char ch = container_[j];
                while(k >= 0 && ch != container_[k])
                    k = p_[k];
                ++k;
                p_.push_back(ch == container_[k] ? p_[k] : k);
            }
        }
    }

    typedef std::vector<int> Ints;
    Ints      p_;
    Container container_;
    
    friend class matcher<Char, Container>;
};

template<class Char=wchar_t, class Container=std::basic_string<Char> >
class matcher {
public:
    typedef pattern<Char, Container> pattern_type;

    explicit matcher(const pattern_type & pattern)
        : len_(0), pattern_(pattern), size_(pattern_.container_.size())
    {
    }

    bool feed(Char ch)
    {
        len_ = pattern_.feed(ch, len_);
        return static_cast<size_t>(len_) == size_;
    }
private:
    int len_;
    const pattern_type & pattern_;
    size_t size_;
};

template<class Iterator, class Char, class Container>
Iterator feed(matcher<Char, Container> & m, Iterator begin, Iterator end)
{
    for(;begin != end; ++begin)
        if(m.feed(*begin))
            break;
    return begin;
}

template<class Iterator, class Char, class Container>
bool check_feed(matcher<Char, Container> & m, Iterator begin, Iterator end)
{
    for(;begin != end; ++begin)
        if(m.feed(*begin))
            return true;
    return false;
}

template<class Cont, class Char, class Container>
typename Cont::const_iterator feed(matcher<Char, Container> & m, const Cont & container)
{
    return feed(m, container.begin(), container.end());
}

template<class Cont, class Char, class Container>
bool check_feed(matcher<Char, Container> & m, const Cont & container)
{
    return check_feed(m, container.begin(), container.end());
}

template<class Cont, class Char, class Container>
typename Cont::const_iterator match(const pattern<Char, Container> & p, const Cont & cont)
{
    matcher<Char, Container> m(p);
    typename Cont::const_iterator result = feed(m, cont);

    if(result != cont.end())
        std::advance(result, -static_cast<int>(p.size() - 1));
    
    return result;
}

template<class Cont, class Char, class Container>
bool check_match(const pattern<Char, Container> & p, const Cont & cont)
{
    matcher<Char, Container> m(p);
    return check_feed(m, cont);
}

}
