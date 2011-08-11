#pragma once

#ifndef MLOG_BUILDING
#include <boost/noncopyable.hpp>
#endif

namespace mlog {

enum LogLevel {
    llEmergency,
    llAlert,
    llCritical,
    llError,
    llWarning,
    llNotice,
    llInfo,
    llDebug,
};

}
