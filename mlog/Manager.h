/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#if defined(_MSC_VER)
#pragma warning(disable: 4396)
#endif

#if !defined(MLOG_BUILDING)

#if !MLOG_NO_LOGGING
#include <boost/intrusive_ptr.hpp>
#include <boost/scoped_ptr.hpp>

#include <mstd/exception.hpp>
#include <mstd/reference_counter.hpp>
#endif

#include <boost/filesystem/path.hpp>

#endif

#include "Defines.h"

namespace mlog {

#if !MLOG_NO_LOGGING
namespace detail {
    class LoggerImpl;
}

typedef std::function<void(uint32_t, LogLevel, const char * logger, const char * msg, size_t len)> Listener;

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

class tag_filename;
typedef boost::error_info<tag_filename, boost::filesystem::path> error_filename;
typedef mstd::own_exception<Manager> ManagerException;
#endif

boost::filesystem::path expandFilePath(const std::string & input);

}
