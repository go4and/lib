/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include <vector>

#include "itoa.hpp"

namespace mstd {

namespace detail {
const char * initBlocks()
{
    static std::vector<char> result(blockLen * blockPow);
    for(size_t i = 0; i != blockPow; ++i)
    {
        result[i * blockLen] = '0' + i % 10;
        result[i * blockLen + 1] = '0' + (i /   10) % 10;
        result[i * blockLen + 2] = '0' + (i /  100) % 10;
        result[i * blockLen + 3] = '0' + (i / 1000) % 10;
    }
    return &result[0];
}

MSTD_DECL const char * const digitBlocks = initBlocks();

}

MSTD_DECL const char * const hex_table = "0123456789abcdef";

}
