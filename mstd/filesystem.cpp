#include <boost/config.hpp>

#if !BOOST_WINDOWS
#include <stdio.h>
#endif

#include <sys/stat.h>

#include "filesystem.hpp"
#include "rc_buffer.hpp"

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

std::streamsize file_size(FILE * file)
{
#if BOOST_WINDOWS
    struct _stat64 stat;
    if(!_fstat64(_fileno(file), &stat))
#else
    struct stat stat;
    if(!fstat(fileno(file), &stat))
#endif
        return stat.st_size;
    else
        return -1;
}

rc_buffer load_file(const boost::filesystem::wpath & path)
{
    FILE * file = wfopen(path, "rb");
    if(file)
    {
        off_t size = file_size(file);
        if(size >= 0)
        {
            rc_buffer result(size);
            off_t read = fread(result.data(), 1, size, file);
            if(read == size)
                return result;
        }
    }

    return rc_buffer();
}

}
