/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

#include <boost/array.hpp>

namespace mstd {

    namespace detail {

        template<size_t N>
        struct get_uint {
            typedef typename boost::mpl::if_c <
                        N < 0x100,
                        boost::uint8_t,
                        typename boost::mpl::if_c <
                            N < 0x10000,
                            boost::uint16_t,
                            boost::uint32_t
                        >::type
                    >::type type;
        };

    }

template<size_t N>
class changed_set {
private:
    typedef typename detail::get_uint<N>::type uint_type;
    typedef std::vector<uint_type> list_type;
public:
    typedef typename list_type::const_iterator iterator;
    typedef typename list_type::const_iterator const_iterator;

    changed_set()
    {
        marked_.assign(false);
    }

    void changed(size_t idx)
    {
        BOOST_ASSERT(idx < N);
        if(!marked_[idx])
        {
            list_.push_back(idx);
            marked_[idx] = true;
        }
    }

    void clear()
    {
        if(list_.size() > N / 4)
            marked_.assign(false);
        else
            for(iterator i = list_.begin(), end = list_.end(); i != end; ++i)
                marked_[*i] = false;
        list_.clear();
    }

    bool empty() const
    {
        return list_.empty();
    }

    size_t size() const
    {
        return list_.size();
    }

    const_iterator begin() const
    {
        return list_.begin();
    }

    const_iterator end() const
    {
        return list_.end();
    }
private:
    boost::array<bool, N> marked_;
    std::vector<uint_type> list_;
};

}
