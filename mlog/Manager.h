#pragma once

#if defined(_MSC_VER)
#pragma warning(disable: 4396)
#endif

#if !defined(MLOG_BUILDING) && !MLOG_NO_LOGGING
#include <boost/function.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/scoped_ptr.hpp>

#include <mstd/exception.hpp>
#include <mstd/reference_counter.hpp>
#endif

#include "Defines.h"

namespace mlog {

#if !MLOG_NO_LOGGING
namespace detail {
    class LoggerImpl;
}

typedef boost::function<void(uint32_t, LogLevel, const char * logger, const char * msg, size_t len)> Listener;

class MLOG_DECL Manager {
    MSTD_SINGLETON_DEFINITION(Manager);
public:
    ~Manager();

    void setAppName(const char * appname);
    void setup(const std::string & expr);
    void output(const char * logger, const Buffer & buf);
    void setListener(LogLevel level, const Listener & listener);
private:
    Manager();

    detail::LoggerImpl & registerLogger(const std::string & name);
    
    class Impl;
    boost::scoped_ptr<Impl> impl_;

    friend class Logger;
};

typedef mstd::own_exception<Manager> ManagerException;
#endif

}
