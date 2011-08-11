#include <boost/config.hpp>

#if !BOOST_WINDOWS
#include <stdio.h>
#endif

#include "filesystem.hpp"

namespace mstd {

FILE * wfopen(const boost::filesystem::wpath & path, const char * mode)
{
#if BOOST_WINDOWS
    wchar_t buf[0x10];
    std::copy(mode, mode + strlen(mode) + 1, buf);
    return _wfopen(mstd::wfname(path).c_str(), buf);
#else
    return fopen(mstd::apifname(path).c_str(), mode);
#endif
}

}
