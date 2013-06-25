/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

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
