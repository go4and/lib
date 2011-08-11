#pragma once

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
