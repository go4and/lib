#pragma once

namespace mnet {

typedef int64_t filesize_t;

enum DownloadAction {
    daFinished,
    daFailed,
    daStopped,
};

}
