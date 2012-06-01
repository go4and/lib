#pragma once

#if !MLOG_NO_LOGGING
#if !defined(MLOG_BUILDING)
#include <iostream>

#include <string>

#include <boost/noncopyable.hpp>

#include <mstd/singleton.hpp>
#endif
#endif

#include "Config.h"

#include "Manager.h"

namespace mlog {

class MLOG_DECL Logger : public boost::noncopyable {
public:
#if !MLOG_NO_LOGGING
    explicit Logger(const std::string & name)
        : name_(name)
        , impl_(Manager::instance().registerLogger(name))
         {}
#endif

#if !MLOG_NO_LOGGING
    const char * name() const
    {
        return name_.c_str();
    }

    mlog::LogLevel level() const
    {
        return impl_.level();
    }

    bool enabled(LogLevel level) const
    {
        return impl_.enabled(level);
    }
    
    boost::uint32_t group() const
    {
        return impl_.group();
    }
    
    void outputHeader(std::ostream & out, LogLevel level);
#endif
private:
#if !MLOG_NO_LOGGING
    std::string name_;
    detail::LoggerImpl & impl_;
#endif
};

#if !MLOG_NO_LOGGING
#define MLOG_DECLARE_LOGGER(name) \
    namespace { \
        class Logger_##name : public mlog::Logger { \
        public: \
            Logger_##name() \
                : Logger(#name) {} \
        private: \
            MSTD_SINGLETON_DECLARATION(Logger_##name); \
        }; \
        mlog::Logger & logger = (mstd::singleton<Logger_##name>::instance()); \
    } \
    /**/
#else
#define MLOG_DECLARE_LOGGER(name) 
#endif

#if !MLOG_NO_LOGGING
MLOG_DECL const std::string & levelName(LogLevel level);
#endif

}
