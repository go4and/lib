/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include "pch.h"

#if !MLOG_NO_LOGGING

#include "Logger.h"
#include "Utils.h"

#include "Manager.h"

namespace mlog {

namespace {

class Data : public mstd::singleton<Data> {
public:
    void appname(const std::string & value)
    {
        appname_ = value;
    }

    const std::string & appname() const
    {
        return appname_;
    }
private:
    std::string appname_;

    MSTD_SINGLETON_DECLARATION(Data);
};

template<class Type>
class Updater {
public:
    typedef void (LogParticipant::*Func)(Type type);

    explicit Updater(Func func, Type value)
        : func_(func), value_(value) {}
        
    template<class D, class I, class J>
    void operator()(D&, I&, J j) const
    {
        ((**j).*func_)(value_);
    }

    template<class T>
    void operator()(T & t) const
    {
        (t.*func_)(value_);
    }
private:
    Func func_;
    Type value_;
};

}

#if defined(__APPLE__)
void nslogWrite(LogLevel level, const char * out, size_t len);
extern std::string documentsFolder();
#endif

namespace {

class CheckName {
public:
    explicit CheckName(const std::string & name)
        : name_(name) {}
    
    template<class F>
    bool operator()(const F & f)
    {
        return f->name() == name_;
    }
private:
    std::string name_;
};

class MLOG_DECL LogDevice : boost::noncopyable, public LogParticipant, public mstd::reference_counter<LogDevice> {
public:
    explicit LogDevice()
        : LogParticipant(llDebug) {}

    void name(const std::string & value)
    {
        name_ = value;
    }

    const std::string & name() const
    {
        return name_;
    }

    void output(LogLevel level, const char * msg, size_t len)
    {
        if(enabled(level))
            doOutput(level, msg, len);
    }

    virtual ~LogDevice() {}
private:
    virtual void doOutput(LogLevel level, const char * msg, size_t len) = 0;

    std::string name_;
};

typedef boost::intrusive_ptr<LogDevice> LogDevicePtr;

class CFileLogDevice : public LogDevice {
public:
    explicit CFileLogDevice(FILE * handle)
        : handle_(handle)
    {
    }
    
    void doOutput(LogLevel level, const char * str, size_t len)
    {
        const size_t limit = 0x400;
        while(len)
        {
            size_t wr = fwrite(str, 1, std::min(limit, len), handle_);
            if(!wr)
            {
                int err = errno;
                if(err == 9)
                    fflush(handle_);
                else
                    break;
            }
            str += wr;
            len -= wr;
        }
        
        fflush(handle_);
    }

    void changeHandle(FILE * value)
    {
        handle_ = value;
    }
private:
    FILE * handle_;
};

#if BOOST_WINDOWS
class VCOutputDevice : public LogDevice {
public:
    VCOutputDevice()
    {
    }

    void doOutput(LogLevel level, const char * out, size_t len)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);
        buffer_.clear();
        mstd::deutf8(out, out + len, std::back_inserter(buffer_));
        buffer_.push_back(0);
        OutputDebugString(&buffer_[0]);
    }
private:
    boost::mutex mutex_;
    std::vector<wchar_t> buffer_;
};
#endif

#if defined(ANDROID)
android_LogPriority androidLogPriority[] =
    { ANDROID_LOG_FATAL, ANDROID_LOG_FATAL, ANDROID_LOG_FATAL,
      ANDROID_LOG_ERROR,
      ANDROID_LOG_WARN,
      ANDROID_LOG_INFO,
      ANDROID_LOG_DEBUG,
      ANDROID_LOG_VERBOSE,
    };

class AndroidLogDevice : public LogDevice {
public:
    AndroidLogDevice(const std::string & tag)
        : tag_(tag)
    {
    }

    void doOutput(LogLevel level, const char * out, size_t len)
    {
        __android_log_write(androidLogPriority[level], tag_.c_str(), out);
    }
private:
    std::string tag_;
};
#endif

#if defined(__APPLE__)
class NSLogDevice : public LogDevice {
public:
    NSLogDevice()
    {
    }

    void doOutput(LogLevel level, const char * out, size_t len)
    {
        nslogWrite(level, out, len);
    }
};
#endif

class NullLogDevice : public LogDevice {
public:
    void doOutput(LogLevel level, const char * out, size_t len)
    {
    }
};

class FClose {
public:
    void operator()(FILE * file)
    {
        if(file != 0)
            fclose(file);
    }    
};

class FileHolder : public boost::noncopyable {
public:
    explicit FileHolder(const boost::filesystem::path & fname)
        : fname_(fname)
    {
        boost::filesystem::wpath dir = fname;
        dir.remove_filename();
        boost::system::error_code ec;
        create_directories(dir, ec);
        handle_ = mstd::wfopen(fname, "ab");
        if(!handle_)
            BOOST_THROW_EXCEPTION(ManagerException() << mstd::error_message("Failed to open for writing") << error_filename(fname));
    }
    
    FILE * handle()
    {
        return handle_;
    }
    
    const boost::filesystem::path & fname() const
    {
        return fname_;
    }

    void close()
    {
        fclose(handle_);
        handle_ = 0;
    }

    void reopen()
    {
        handle_ = mstd::wfopen(fname_, "wb");
        if(!handle_)
            BOOST_THROW_EXCEPTION(ManagerException() << mstd::error_message("Failed to open for writing") << error_filename(fname_));
    }

    ~FileHolder()
    {
        if(handle_)
            fclose(handle_);
    }
private:
    boost::filesystem::path fname_;
    FILE * handle_;
};

#if defined(BOOST_WINDOWS)
DWORD getpid()
{
    return GetCurrentProcessId();
}
#endif

#if BOOST_WINDOWS
std::string documentsFolder()
{
    wchar_t buf[MAX_PATH + 1];
    HRESULT hr = SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, 0, buf);
    if(SUCCEEDED(hr))
        return mstd::utf8(buf);
    const wchar_t * b = _wgetenv(L"USERPROFILE");
    if(b)
        return mstd::utf8fname(boost::filesystem::wpath(b) / L"My Documents");
    else
        return "My Documents";
}
#endif

std::string parse(const std::string & fname)
{
    std::string result;
    result.reserve(fname.length());
    std::string::const_iterator end = fname.end(), p = end;
    for(std::string::const_iterator i = fname.begin(); i != end; ++i)
    {
        char c = *i;
        if(c == '%')
        {
            if(p == end)
                p = i;
            else {
                std::string key = std::string(p + 1, i);
                if(key.empty())
                    result.push_back('%');
                else if(key == "pid")
                {
                    char buf[0x20];
                    result += mstd::itoa(getpid(), buf);
                } else if(key == "app")
                    result += Data::instance().appname();
                else if(key == "now")
                {
                    std::string temp = boost::lexical_cast<std::string>(onow());
                    std::replace(temp.begin(), temp.end(), ':', '-');
                    result += temp;
                } 
#if BOOST_WINDOWS || defined(__APPLE__)
                else if(key == "documents")
                    result += documentsFolder();
#endif
                else
                    BOOST_THROW_EXCEPTION(ManagerException() << mstd::error_message("Unknown key: " + key));
                p = end;
            }
        } else if(p == end)
            result.push_back(c);
    }

    return result;
}

class FileLogDevice : private FileHolder, public CFileLogDevice {
public:
    explicit FileLogDevice(const boost::filesystem::path & fname, size_t threshold, size_t count)
        : FileHolder(fname), CFileLogDevice(handle()), threshold_(threshold), count_(count)
    {
        current_ = ftell(handle());
        checkLimit();

        std::ostringstream out;
        out << "============== NEW LOG STARTED at ";
        out << boost::posix_time::microsec_clock::local_time();
        out << " ==============" << std::endl;
        std::string message = out.str();
        doOutput(llNotice, message.c_str(), message.length());
    }
    
    void doOutput(LogLevel level, const char * out, size_t len)
    {
        boost::lock_guard<boost::mutex> guard(mutex_);
        CFileLogDevice::doOutput(level, out, len);
        current_ += len;
        checkLimit();
    }
private:
    void checkLimit()
    {
        if(threshold_ && current_ >= threshold_)
        {
            close();
            if(count_)
            {
                try {
                    boost::filesystem::path c;
                    size_t i = 0;
                    for(; i != count_; ++i)
                    {
                        c = history(i);
                        if(!exists(c))
                            break;
                    }
                    if(i == count_)
                    {
                        c = history(0);
                        remove(c);
                        for(size_t i = 1; i != count_; ++i)
                        {
                            boost::filesystem::path next(history(i));
                            rename(next, c);
                            c = next;
                        }
                    }
                    rename(fname(), c);
                } catch(boost::system::system_error &) {
                }
            }
            reopen();
            changeHandle(handle());
            current_ = 0;
        }
    }

    boost::filesystem::path history(size_t idx) const
    {
        boost::filesystem::path::value_type buf[0x20];
        return fname().native() + static_cast<boost::filesystem::path::value_type>('.') + mstd::itoa(idx, buf);
    }

    boost::mutex mutex_;
    size_t threshold_;
    size_t current_;
    size_t count_;
};

LogDevice * createDevice(const std::string & name, const std::string & value)
{
    LogDevice * device = 0;
    if(value == "stdout")
        device = new CFileLogDevice(stdout);
    else if(value == "stderr")
        device = new CFileLogDevice(stderr);
#if BOOST_WINDOWS
    else if(value == "vcoutput")
        device = new VCOutputDevice();
#endif
    else if(value == "null")
        device = new NullLogDevice();
    else if(boost::starts_with(value, "file("))
    {
        if(value[value.length() - 1] != ')')
            BOOST_THROW_EXCEPTION(ManagerException() << mstd::error_message("File device syntax: file(filename)"));
        std::string args = value.substr(5, value.length() - 6);
        std::string::size_type p = args.find(',');
        size_t threshold = std::numeric_limits<size_t>::max();
        if(p != std::string::npos)
        {
            threshold = mstd::str2int10<size_t>(args.substr(p + 1));
            args.erase(p);
            boost::trim(args);
        }
        device = new FileLogDevice(mstd::expand_env_vars(parse(args)), threshold, 5);
    }
#if defined(__APPLE__)
    else if(value == "nslog()")
    {
        device = new NSLogDevice();
    }
#endif
#if defined(ANDROID)
    else if(boost::starts_with(value, "android_log("))
    {
        if(value[value.length() - 1] != ')')
            BOOST_THROW_EXCEPTION(ManagerException() << mstd::error_message("Android log device syntax: android_log(tag)"));
        device = new AndroidLogDevice(value.substr(12, value.length() - 13));
    }
#endif
    else
        BOOST_THROW_EXCEPTION(ManagerException() << mstd::error_message("Unknown log device: " + value));
    
    device->name(name);
    return device;
}

struct EraseDevice {
    template<class D, class I, class J>
    void operator()(D&, I i, J j) const
    {
        i->erase(j);
    }
};

class ChangeDevice {
public:
    explicit ChangeDevice(const LogDevicePtr & device)
        : device_(device) {}

    template<class D, class I, class J>
    void operator()(D&, I i, J j) const
    {
        *j = device_;
    }
private:
    LogDevicePtr device_;
};

class UpdateGroup {
public:
    explicit UpdateGroup(boost::uint32_t group)
        : group_(group) {}

    template<class D, class I, class J>
    void operator()(D& d, I i, J j) const
    {
        if(static_cast<boost::uint32_t>(i - d.begin()) != group_)
        {
            typename std::iterator_traits<J>::value_type t = *j;
            i->erase(j);
            if(d.size() <= group_)
                d.resize(group_ + 1);
            d[group_].push_back(t);
        }
    }

    void operator()(detail::LoggerImpl & logger) const
    {
        logger.group(group_);
    }
private:
    boost::uint32_t group_;
};

typedef std::vector<LogDevicePtr> DeviceGroup;

class Devices : boost::noncopyable, public mstd::reference_counter<Devices> {
public:
    explicit Devices(bool withConsole)
    {
        if(withConsole)
        {
            DeviceGroup dg;
            LogDevicePtr device(new CFileLogDevice(stdout));
            device->name("console");
            dg.push_back(device);
            groups_.push_back(dg);
        } else
            groups_.resize(1);
    }

    template<class F>
    bool setup(const std::string & name, const F & f, boost::mutex::scoped_lock & lock)
    {
        for(std::vector<DeviceGroup>::iterator i = groups_.begin(); i != groups_.end(); ++i)
            for(DeviceGroup::iterator j = i->begin(); j != i->end(); ++j)
                if((*j)->name() == name)
                {
                    f(groups_, i, j);
                    return true;
                }
        return false;
    }

    void append(const LogDevicePtr & device)
    {
        groups_.resize(std::max(static_cast<size_t>(1), groups_.size()));
        groups_[0].push_back(device);
    }

    inline void output(uint32_t group, LogLevel level, const char * p, size_t len)
    {
        DeviceGroup & devs = groups_[0];
        DeviceGroup::iterator i = devs.begin();
        DeviceGroup::iterator end = devs.end();
        for(; i != end; ++i)
        {
            LogDevice & device = **i;
            device.output(level, p, len);
        }

        if(group && group < groups_.size())
        {
            DeviceGroup & devs = groups_[group];
            DeviceGroup::iterator i = devs.begin();
            DeviceGroup::iterator end = devs.end();
            for(; i != end; ++i)
            {
                LogDevice & device = **i;
                device.output(level, p, len);
            }
        }
    }
    
    Devices * clone()
    {
        return new Devices(groups_);
    }
private:
    explicit Devices(const std::vector<DeviceGroup> & groups)
    {
        groups_ = groups;
    }

    std::vector<DeviceGroup> groups_;
};

typedef boost::intrusive_ptr<Devices> DevicesPtr;

}

class Manager::Impl {
public:
    Impl()
        : rootLogger_(llWarning), devices_(new Devices(true)), threadStarted_(false), realtime_(false)
    {
    }

    ~Impl()
    {
        if(threadStarted_)
        {
            for(;;)
            {
                {
                    boost::lock_guard<boost::mutex> lock(mutex_);
                    if(queue_.empty())
                        break;
                }
                boost::this_thread::yield();
            }
            thread_.interrupt();
            cond_.notify_one();
            thread_.join();
        }
    }

    void setListener(LogLevel level, const Listener & listener)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);
        listenerLevel_ = level;
        listener_ = listener;
    }

    void setup(const std::string & expr)
    {
        std::string::size_type dp = expr.find('.');
        if(dp == std::string::npos)
            BOOST_THROW_EXCEPTION(ManagerException() << mstd::error_message("Invalid setup syntax - '.' expected"));
        std::string::size_type ep = expr.find('=', dp);
        if(ep == std::string::npos)
            BOOST_THROW_EXCEPTION(ManagerException() << mstd::error_message("Invalid setup syntax - '=' expected"));
        std::string name = expr.substr(0, dp);
        boost::trim(name);
        std::string prop = expr.substr(dp + 1, ep - dp - 1);
        boost::trim(prop);
        std::string value = expr.substr(ep + 1);
        boost::trim(value);
        
        boost::mutex::scoped_lock lock(mutex_);
        if(name == "device")
            setupDevice(prop, value, lock);
        else if(name == "manager")
        {
            if(prop == "realtime")
            {
                if(value == "1" || value == "true")
                    realtime_ = true;
                else if(value == "0" || value == "false")
                    realtime_ = false;
                else
                    BOOST_THROW_EXCEPTION(ManagerException() << mstd::error_message("Invalid realtime value: " + value));
            } else
                BOOST_THROW_EXCEPTION(ManagerException() << mstd::error_message("Unknown manager prop: " + prop));
        } else if(prop == "level")
            setupLevel(name, value, lock);
        else if(prop == "group")
            setupGroup(name, value, lock);
        else
            BOOST_THROW_EXCEPTION(ManagerException() << mstd::error_message("Unknown command: " + expr));
    }

    void output(const char * logger, const Buffer & buf)
    {
        if(realtime_)
        {
            boost::lock_guard<boost::mutex> lock(mutex_);
            queue_.push_back(Queue::value_type(logger, buf));
            cond_.notify_one();
            if(!threadStarted_)
            {
                threadStarted_ = true;
                thread_ = boost::thread(&Impl::execute, this);
            }
        } else {
            DevicesPtr devices;
            {
                boost::lock_guard<boost::mutex> lock(mutex_);
                devices = devices_;
            }
            Listener listener;
            bool listenerChecked = false;
            process(*devices, logger, buf, listener, listenerChecked);
        }
    }

    detail::LoggerImpl & registerLogger(const std::string & name)
    {
        boost::mutex::scoped_lock lock(mutex_);
        return registerLogger(name, lock);
    }

    detail::LoggerImpl & registerLogger(const std::string & name, boost::mutex::scoped_lock & lock)
    {
        Loggers::iterator i = loggers_.find(name);
        if(i == loggers_.end())
            i = loggers_.insert(Loggers::value_type(name, detail::LoggerImpl(rootLogger_.level()))).first;
        
        return const_cast<detail::LoggerImpl&>(i->second);
    }
private:
    typedef boost::unordered_map<std::string, detail::LoggerImpl> Loggers;
    typedef std::vector<std::pair<const char *, Buffer> > Queue;

    void setupGroup(const std::string & name, const std::string & value, boost::mutex::scoped_lock & lock)
    {
        uint32_t group = mstd::str2int10<uint32_t>(value);

        if(name.empty())
        {
            rootLogger_.group(group);
            BOOST_FOREACH(const Loggers::value_type & i, loggers_)
                const_cast<detail::LoggerImpl&>(i.second).group(group);
        } else
            registerLogger(name, lock).group(group);

        setup(name, UpdateGroup(group), lock);
    }

    void setupDevice(const std::string & prop, const std::string & value, boost::mutex::scoped_lock & lock)
    {
        DevicesPtr newDevices(devices_->clone());
        if(prop == "remove")
        {
            if(value == "*")
            {
                Devices * devices = new Devices(false);
                newDevices.reset(devices);
            } else {
                if(!newDevices->setup(value, EraseDevice(), lock))
                    BOOST_THROW_EXCEPTION(ManagerException() << mstd::error_message("Unknown log device: " + value));
            }
        } else {
            LogDevicePtr newDevice(createDevice(prop, value));

            if(!newDevices->setup(prop, ChangeDevice(newDevice), lock))
                newDevices->append(newDevice);
        }

        devices_ = newDevices;
    }

    void process(Devices & devices, const char * logger, const Buffer & buf, Listener & listener, bool & listenerChecked)
    {
        char * p = bufferData(buf);
        uint32_t group = *mstd::pointer_cast<uint32_t*>(p);
        p += sizeof(group);
        LogLevel level = *mstd::pointer_cast<LogLevel*>(p);
        p += sizeof(level);
        size_t len = *mstd::pointer_cast<size_t*>(p);
        p += sizeof(len);

        devices.output(group, level, p, len);

        if(level <= listenerLevel_)
        {
            if(!listenerChecked)
            {
                boost::lock_guard<boost::mutex> lock(mutex_);
                listener = listener_;
                listenerChecked = true;
            }
            if(!listener.empty())
                listener(group, level, logger, p, len);
        }
    }

    void execute()
    {
        Queue queue;
        try {
            boost::unique_lock<boost::mutex> lock(mutex_);
            while(!boost::this_thread::interruption_requested())
            {
                if(queue_.empty())
                    cond_.wait(lock);
                else {
                    queue.swap(queue_);
                    DevicesPtr devices = devices_;
                    mstd::reverse_lock<boost::unique_lock<boost::mutex> > rlock(lock);
                    Listener listener;
                    bool listenerChecked = false;
                    for(Queue::const_iterator q = queue.begin(), qend = queue.end(); q != qend; ++q)
                        process(*devices, q->first, q->second, listener, listenerChecked);
                    queue.clear();
                }
            }
        } catch(boost::thread_interrupted&) {
        }
    }    

    template<class F>
    void setup(const std::string & name, const F & f, boost::mutex::scoped_lock & lock)
    {
        if(name.empty())
        {
            f(rootLogger_);
            BOOST_FOREACH(const Loggers::value_type & i, loggers_)
                f(const_cast<detail::LoggerImpl&>(i.second));
        } else {
            DevicesPtr newDevices(devices_->clone());
            if(newDevices->setup(name, f, lock))
                devices_ = newDevices;
            else
                f(registerLogger(name, lock));
        }
    }

    void setupLevel(const std::string & name, const std::string & value, boost::mutex::scoped_lock & lock)
    {
        LogLevel level;
        if(value == "emergency")
            level = llEmergency;
        else if(value == "alert")
            level = llAlert;
        else if(value == "critical")
            level = llCritical;
        else if(value == "error")
            level = llError;
        else if(value == "warning")
            level = llWarning;
        else if(value == "notice")
            level = llNotice;
        else if(value == "info")
            level = llInfo;
        else if(value == "debug")
            level = llDebug;
        else {
            BOOST_THROW_EXCEPTION(ManagerException() << mstd::error_message("Unknown log level: " + value));
            std::terminate();
        }

        setup(name, Updater<LogLevel>(&detail::LoggerImpl::level, level), lock);
    }

    detail::LoggerImpl rootLogger_;
    Loggers loggers_;
    DevicesPtr devices_;
    boost::mutex mutex_;
    boost::condition_variable cond_;
    LogLevel listenerLevel_;
    Listener listener_;
    Queue queue_;
    boost::thread thread_;
    bool threadStarted_;
    bool realtime_;
};

void Manager::setup(const std::string & expr)
{
    impl_->setup(expr);
}

void Manager::setListener(LogLevel level, const Listener & listener)
{
    impl_->setListener(level, listener);
}

void Manager::output(const char * logger, const Buffer & buf)
{
    impl_->output(logger, buf);
}
    
detail::LoggerImpl & Manager::registerLogger(const std::string & name)
{
    return impl_->registerLogger(name);
}

void Manager::setAppName(const char * value)
{
    Data::instance().appname(value);
}

Manager::Manager()
    : impl_(new Impl)
{
#if MLOG_USE_BUFFERS
    mstd::buffers::instance();
#endif
    levelName(llError);
}

Manager::~Manager()
{
    impl_.reset();
}

MSTD_SINGLETON_IMPLEMENTATION(Manager);

}

#endif
