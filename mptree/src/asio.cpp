#include "pch.hpp"

#include "../asio.hpp"

namespace mptree {

bool parse_value(mptree::subnet & out, const char * value, size_t len)
{
    try {
        mptree::subnet temp = { 0, 0 };
        const char * end = value + len;
        for(size_t i = 0; i != 4; ++i)
        {
            const char * p = std::find(value, end, i != 3 ? '.' : '/');
            if(p == end)
                return false;
            temp.address |= mstd::str2int10_checked<uint32_t>(value, p) << (8 * (3 - i));
            value = p + 1;
        }
        temp.masklen = mstd::str2int10_checked<uint32_t>(value, end);
        if(temp.masklen > 32 || (temp.address & temp.mask()) != 0)
            return false;
        out = temp;
        return true;
    } catch(mstd::bad_str2int_cast &) {
        return false;
    }
}

size_t render_short_value(char * out, const mptree::subnet & value)
{
    char * p = out;
    for(size_t i = 0; i != 4; ++i)
    {
        mstd::itoa((value.address >> (8 * (3 - i))) & 0xff, p);
        p += strlen(p);
        *p++ = i != 3 ? '.' : '/';
    }
    mstd::itoa(value.masklen, p);
    p += strlen(p);
    return p - out;
}

}
