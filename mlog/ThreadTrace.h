/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#if !defined(MLOG_BUILDING) && !MLOG_NO_LOGGING
#include <exception>

#include <boost/thread/exceptions.hpp>
#endif

#include "Config.h"

namespace boost {
    class exception;
    class thread_interrupted;
}

namespace mlog {

class Logger;

class MLOG_DECL TracerBase {
protected:
    TracerBase(Logger & logger, bool restart);

    void logInterruption();
    void logBoostException(boost::exception & exc);
    void logStdException(std::exception & exc);
    void logUnknownException();
    bool restart() const;
    bool interruptionRequested() const;
private:
    Logger & logger_;
    bool restart_;
};

template<class Handler>
class Tracer : private TracerBase {
public:
    explicit Tracer(Logger & logger, const Handler & handler, bool restart = false)
        : TracerBase(logger, restart), handler_(handler) {}

    void operator()()
    {
        bool go = true;
        while(!interruptionRequested() && go)
            try {
                go = restart();
                handler_();
            } catch(boost::thread_interrupted&) {
                logInterruption();
                break;
            } catch(boost::exception & exc) {
                logBoostException(exc);
            } catch(std::exception & exc) {
                logStdException(exc);
            } catch(...) {
                logUnknownException();
            }
    }
private:
    Handler handler_;
};

template<class Handler>
Tracer<Handler> tracer(Logger & logger, const Handler & handler, bool restart = false)
{
    return Tracer<Handler>(logger, handler, restart);
}

}
