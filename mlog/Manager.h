#pragma once

#if defined(_MSC_VER)
#pragma warning(disable: 4396)
#endif

#if !defined(MLOG_BUILDING) && !MLOG_NO_LOGGING
#include <vector>

#include <boost/function.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>

#include <mstd/atomic.hpp>
#include <mstd/cstdint.hpp>

#include "Config.h"

#if MLOG_USE_BUFFERS
#include <mstd/buffers.hpp>
#else
#include <mstd/rc_buffer.hpp>
#endif

#include <mstd/exception.hpp>
#include <mstd/reference_counter.hpp>
#include <mstd/singleton.hpp>
#endif

#include "Defines.h"

namespace mlog {

#if !MLOG_NO_LOGGING
class MLOG_DECL LogParticipant {
public:
    LogParticipant(LogLevel level)
        : level_(level) {}

    LogLevel level() const
    {
        return level_;
    }
    
    void level(LogLevel level)
    {
        level_ = level;
    }
    
    bool enabled(LogLevel level) const
    {
        return level <= level_;
    }
private:
    mstd::atomic<LogLevel> level_;
};

namespace detail {
    class MLOG_DECL LoggerImpl : public LogParticipant {
    public:
        LoggerImpl(LogLevel level)
            : LogParticipant(level), group_(0) {}

        boost::uint32_t group() const
        {
            return group_;
        }

        void group(boost::uint32_t group)
        {
            group_ = group;
        }
    private:
        mstd::atomic<boost::uint32_t> group_;
    };
}

class Logger;

class MLOG_DECL LogDevice : public LogParticipant, public mstd::reference_counter<LogDevice> {
public:
    typedef boost::function<void(LogLevel level, const char * msg, size_t len)> Device;

    explicit LogDevice(const std::string & name, const Device & device)
        : LogParticipant(llDebug), name_(name), device_(device) {}

    const std::string & name() const
    {
        return name_;
    }

    void output(LogLevel level, const char * msg, size_t len)
    {
        if(enabled(level))
            device_(level, msg, len);
    }
private:
    std::string name_;
    Device device_;
};

typedef boost::intrusive_ptr<LogDevice> LogDevicePtr;

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

    void setupLevel(const std::string & name, const std::string & value, boost::unique_lock<boost::mutex> & lock);
    void setupGroup(const std::string & name, const std::string & value, boost::unique_lock<boost::mutex> & lock);
    void setupDevice(const std::string & prop, const std::string & value, boost::unique_lock<boost::mutex> & lock);
    void execute();

    template<class F>
    void setup(const std::string & name, const F & f, boost::unique_lock<boost::mutex> & lock);

    detail::LoggerImpl & registerLogger(const std::string & name);
    detail::LoggerImpl & registerLogger(const std::string & name, boost::unique_lock<boost::mutex> & lock);

    friend class Logger;

    typedef boost::unordered_map<std::string, detail::LoggerImpl> Loggers;
    typedef std::vector<LogDevicePtr> DeviceGroup;
    typedef std::vector<DeviceGroup> Devices;
    typedef boost::shared_ptr<Devices> SharedDevices;
    typedef std::vector<std::pair<const char *, Buffer> > Queue;

    void process(Devices & devices, const char * logger, const Buffer & buffer, Listener & listener, bool & listenerChecked);

    detail::LoggerImpl rootLogger_;
    Loggers loggers_;
    SharedDevices devices_;
    boost::mutex mutex_;
    boost::condition_variable cond_;
    LogLevel listenerLevel_;
    Listener listener_;
    Queue queue_;
    boost::thread thread_;
    bool threadStarted_;
    bool realtime_;
};

typedef mstd::own_exception<Manager> ManagerException;
#endif

}
