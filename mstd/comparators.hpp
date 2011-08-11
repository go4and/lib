#pragma once

namespace mstd {

struct compare_by_first {
    template<class T>
    bool operator()(const T & lhs, const T & rhs) const
    {
        return lhs.first > rhs.first;
    }
};

}
