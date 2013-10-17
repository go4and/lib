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

struct compare_by_first {
    template<class T>
    bool operator()(const T & lhs, const T & rhs) const
    {
        return lhs.first > rhs.first;
    }
};

}
