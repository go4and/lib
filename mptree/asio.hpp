#pragma once

namespace mptree {

struct subnet {
    uint32_t address;
    uint32_t masklen;

    uint32_t mask() const
    {
        return masklen ? (1 << (32 - masklen)) - 1 : 0xffffffff;
    }
};

bool parse_value(mptree::subnet & out, const char * value, size_t len);
size_t render_short_value(char * out, const mptree::subnet & value);

}
