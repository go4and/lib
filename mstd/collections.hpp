/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

#include <vector>

namespace mstd {

template<class Col>
void set_at(Col & col, size_t idx, const typename Col::value_type & value)
{
    if(col.size() <= idx)
        col.resize(idx + 1);
    col[idx] = value;
}

}
