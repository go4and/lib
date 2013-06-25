/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

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
