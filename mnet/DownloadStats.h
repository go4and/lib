#pragma once

#include "DownloadDefines.h"

namespace mnet {

struct DownloadStats {
    filesize_t downloaded;
    filesize_t actualFileSize;
    filesize_t speed;

    boost::posix_time::time_duration elapsed;

    int progress() const { return actualFileSize > 0 ? (std::min)(static_cast<int>((downloaded * 100.) / actualFileSize), 100) : 0; }
};

}
