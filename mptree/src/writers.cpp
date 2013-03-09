#include "pch.hpp"

#include "writers.hpp"

namespace mptree {

size_t render_short_value(char * out, double value)
{
    return snprintf(out, 0x40 - 1, "%g", value);
}

}
