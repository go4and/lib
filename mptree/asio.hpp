#pragma once

namespace mptree {

struct subnet {
    uint32_t address;
    uint32_t masklen;
};

bool parse_value(mptree::subnet & out, const char * value, size_t len);
size_t render_short_value(char * out, const mptree::subnet & value);

}
