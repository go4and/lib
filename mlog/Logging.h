/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

#include "Logger.h"
#include "Manager.h"
#include "Output.h"

namespace mlog {

#if !MLOG_NO_LOGGING
#define MLOG_DO_OUT(level, value) \
    do { \
        mlog::Output _mlog_output; \
        logger.outputHeader(_mlog_output.out(), level); \
        _mlog_output.out() << value; \
        _mlog_output.send(logger.group(), level, logger.name()); \
    } while(false)

#define MLOG_MESSAGE_EX(level, value) \
    do { \
        if(logger.enabled(level)) \
            MLOG_DO_OUT(level, value); \
    } while(false)

#define MLOG_MESSAGE(level, value) MLOG_MESSAGE_EX(mlog::ll##level, value)

#define MLOG_MESSAGE2(level1, value1, level2, value2) \
    do { \
        if(logger.enabled(mlog::ll##level1)) \
            MLOG_DO_OUT(mlog::ll##level1, value1); \
        else if(logger.enabled(mlog::ll##level2)) \
            MLOG_DO_OUT(mlog::ll##level2, value2); \
    } while(false)

#define MLOG_FMESSAGE(level, value) \
    do { \
        mlog::Logger & logger = getLogger(); \
        if(logger.enabled(mlog::ll##level)) \
        { \
            mlog::Output _mlog_output; \
            logger.outputHeader(_mlog_output.out(), mlog::ll##level); \
            _mlog_output.out() << value; \
            _mlog_output.send(logger.group(), mlog::ll##level, logger.name()); \
        } \
    } while(false)
#else
#define MLOG_DO_OUT(level, value) \
    do {} while(false)

#define MLOG_MESSAGE(level, value) \
    do {} while(false)

#define MLOG_MESSAGE2(level1, value1, level2, value2) \
    do {} while(false)

#define MLOG_FMESSAGE(level, value) \
    do {} while(false)

#define MLOG_MESSAGE_EX(level, value) \
    do {} while(false)
#endif

#define MLOG_DEBUG(value) MLOG_MESSAGE(Debug, value)
#define MLOG_INFO(value) MLOG_MESSAGE(Info, value);
#define MLOG_NOTICE(value) MLOG_MESSAGE(Notice, value)
#define MLOG_WARNING(value) MLOG_MESSAGE(Warning, value)
#define MLOG_ERROR(value) MLOG_MESSAGE(Error, value)
#define MLOG_CRITICAL(value) MLOG_MESSAGE(Critical, value)
#define MLOG_ALERT(value) MLOG_MESSAGE(Alert, value)
#define MLOG_EMERGENCY(value) MLOG_MESSAGE(Emergency, value)

}
