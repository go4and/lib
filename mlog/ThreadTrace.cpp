#include "pch.h"

#if !MLOG_NO_LOGGING

#include "Logging.h"

#include "ThreadTrace.h"

namespace mlog {

TracerBase::TracerBase(Logger & logger, bool restart)
    : logger_(logger), restart_(restart) {}

void TracerBase::logInterruption()
{
    Logger & logger = logger_;
    MLOG_MESSAGE(Notice, "Thread interrupted");
}

void TracerBase::logBoostException(boost::exception & exc)
{
    Logger & logger = logger_;
    MLOG_MESSAGE(Critical, "UNEXPECTED EXCEPTION: " << mstd::out_exception(exc));
}

void TracerBase::logStdException(std::exception & exc)
{
    Logger & logger = logger_;
    MLOG_MESSAGE(Critical, "UNEXPECTED EXCEPTION: " << mstd::out_exception(exc));
}

void TracerBase::logUnknownException()
{
    Logger & logger = logger_;
    MLOG_MESSAGE(Critical, "UNKNOWN EXCEPTION");
}

bool TracerBase::restart() const
{
    return restart_;
}

bool TracerBase::interruptionRequested() const
{
    return boost::this_thread::interruption_requested();
}

}

#endif