#include "src/pch.hpp"

#include "writers.hpp"

namespace mptree {

size_t render_short_value(char * out, double value)
{
#if !BOOST_WINDOWS
    return snprintf(out, 0x40 - 1, "%g", value);
#else
    return _snprintf(out, 0x40 - 1, "%g", value);
#endif
}

}
