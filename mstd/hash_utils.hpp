#pragma once

#include <Windows.h>

#include "strings.hpp"

namespace mstd {

void hash_combine(std::size_t& seed, size_t hash)
{
    seed ^= hash + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

inline wchar_t toupper(wchar_t ch)
{
    return reinterpret_cast<wchar_t>(CharUpperW(reinterpret_cast<LPWSTR>(ch)));
}

template<class Ch>
class ic_hasher {
public:
    explicit ic_hasher() {}

    template <class It>
    inline std::size_t hash_range(It first, It last) const
    {
        size_t len = last - first;
        if(len)
        {
            buf_.resize(len);
            Ch * begin = &buf_[0];
            memcpy(begin, &*first, len * sizeof(Ch));
            mstd::to_upper(begin, len);
            std::size_t seed = 0;
            boost::hash_range(seed, begin, begin + len);
            return seed;
        } else
            return 0;
    }

    template<class Range>
    std::size_t operator()(const Range & str) const
    {
        return hash_range(boost::begin(str), boost::end(str));
    }
private:
    mutable std::vector<Ch> buf_;
};

class ic_equal {
public:
    template<class Range>
    bool operator()(const Range & lhs, const Range & rhs) const
    {
        return iequals(lhs, rhs);
    }
};

}
