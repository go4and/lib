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

}

template<class Devices, class F>
bool setupDevice(Devices & devices, const std::string & name, const F & f, boost::mutex::scoped_lock & lock)
{
    for(typename Devices::iterator i = devices.begin(); i != devices.end(); ++i)
        for(typename Devices::value_type::iterator j = i->begin(); j != i->end(); ++j)
            if((*j)->name() == name)
            {
                f(devices, i, j);
                return true;
            }
    return false;
}

void Manager::setup(const std::string & expr)
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
            realtime_ = value == "1" || value == "true";
    } else if(prop == "level")
        setupLevel(name, value, lock);
    else if(prop == "group")
        setupGroup(name, value, lock);
    else
        BOOST_THROW_EXCEPTION(ManagerException() << mstd::error_message("Unknown command: " + expr));
}

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

template<class F>
void Manager::setup(const std::string & name, const F & f, boost::mutex::scoped_lock & lock)
{
    if(name.empty())
    {
        f(rootLogger_);
        BOOST_FOREACH(const Loggers::value_type & i, loggers_)
            f(const_cast<detail::LoggerImpl&>(i.second));
    } else {
        SharedDevices newDevices(new Devices(*devices_));
        if(mlog::setupDevice(*newDevices, name, f, lock))
            devices_ = newDevices;
        else
            f(registerLogger(name, lock));
    }
}

void Manager::setupLevel(const std::string & name, const std::string & value, boost::mutex::scoped_lock & lock)
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

void Manager::setupGroup(const std::string & name, const std::string & value, boost::mutex::scoped_lock & lock)
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

}

class CFileLogDevice {
public:
    explicit CFileLogDevice(FILE * handle)
        : handle_(handle)
    {
    }
    
    void operator()(LogLevel level, const char * str, size_t len)
    {
        size_t wr = fwrite(str, 1, len, handle_);
        BOOST_VERIFY(wr == len);
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
class VCOutputDevice {
public:
    VCOutputDevice()
        : mutex_(new boost::mutex)
    {
    }

    void operator()(LogLevel level, const char * out, size_t len)
    {
        boost::lock_guard<boost::mutex> lock(*mutex_);
        buffer_.clear();
        mstd::deutf8(out, out + len, std::back_inserter(buffer_));
        buffer_.push_back(0);
        OutputDebugString(&buffer_[0]);
    }
private:
    boost::shared_ptr<boost::mutex> mutex_;
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

class AndroidLogDevice {
public:
    AndroidLogDevice(const std::string & tag)
        : tag_(tag)
    {
    }

    void operator()(LogLevel level, const char * out, size_t len)
    {
        __android_log_write(androidLogPriority[level], tag_.c_str(), out);
    }
private:
    std::string tag_;
};
#endif

class SyslogLogDevice {
public:
    void operator()(LogLevel level, const char * out, size_t len)
    {
    }
};

class NullLogDevice {
public:
    void operator()(LogLevel level, const char * out, size_t len)
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
    explicit FileHolder(const std::string & fname)
        : fname_(fname)
    {
        boost::filesystem::wpath path(mstd::deutf8(fname));
        boost::filesystem::wpath dir = path;
        dir.remove_filename();
        boost::system::error_code ec;
        create_directories(dir, ec);
        handle_ = mstd::wfopen(path, "ab");
        if(!handle_)
            BOOST_THROW_EXCEPTION(ManagerException() << mstd::error_message("Failed to open for writing: " + fname));
    }
    
    FILE * handle()
    {
        return handle_;
    }
    
    const std::string & fname() const
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
        handle_ = fopen(fname_.c_str(), "wb");
        if(!handle_)
            BOOST_THROW_EXCEPTION(ManagerException() << mstd::error_message("Failed to open for writing: " + fname_));
    }

    ~FileHolder()
    {
        if(handle_)
            fclose(handle_);
    }
private:
    std::string fname_;
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
#elif defined(__APPLE__)
extern std::string documentsFolder();
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

class FileLogDevice : private FileHolder, private CFileLogDevice {
public:
    explicit FileLogDevice(const std::string & fname, size_t threshold, size_t count)
        : FileHolder(parse(fname)), CFileLogDevice(handle()), threshold_(threshold), count_(count)
    {
        current_ = ftell(handle());
        checkLimit();

        std::ostringstream out;
        out << "============== NEW LOG STARTED at ";
        out << boost::posix_time::microsec_clock::local_time();
        out << " ==============" << std::endl;
        std::string message = out.str();
        (*this)(llNotice, message.c_str(), message.length());
    }
    
    void operator()(LogLevel level, const char * out, size_t len)
    {
        boost::lock_guard<boost::mutex> guard(mutex_);
        CFileLogDevice::operator ()(level, out, len);
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
        char buf[0x20];
        return fname() + "." + mstd::itoa(idx, buf);
    }

    boost::mutex mutex_;
    size_t threshold_;
    size_t current_;
    size_t count_;
};

template<class F>
class SharedDevice {
public:
    explicit SharedDevice(F * f)
        : f_(f) {}

    void operator()(LogLevel level, const char * out, size_t len) const
    {
        (*f_)(level, out, len);
    }
private:
    boost::shared_ptr<F> f_;
};

template<class F>
SharedDevice<F> shared(F * f)
{
    return SharedDevice<F>(f);
}

LogDevice * createDevice(const std::string & name, const std::string & value)
{
    LogDevice::Device device;
    if(value == "stdout")
        device = CFileLogDevice(stdout);
    else if(value == "stderr")
        device = CFileLogDevice(stderr);
    else if(value == "syslog")
        device = SyslogLogDevice();
#if BOOST_WINDOWS
    else if(value == "vcoutput")
        device = VCOutputDevice();
#endif
    else if(value == "null")
        device = NullLogDevice();
    else if(boost::starts_with(value, "file("))
    {
        if(value[value.length() - 1] != ')')
            BOOST_THROW_EXCEPTION(ManagerException() << mstd::error_message("File device syntax: file(filename)"));
        std::string args = value.substr(5, value.length() - 6);
        std::string::size_type p = args.find(',');
        size_t threshold = 1 << 20;
        if(p != std::string::npos)
        {
            threshold = mstd::str2int10<size_t>(args.substr(p + 1));
            args.erase(p);
            boost::trim(args);
        }
        device = shared(new FileLogDevice(args, threshold, 5));
    }
#if defined(ANDROID)
    else if(boost::starts_with(value, "android_log("))
    {
        if(value[value.length() - 1] != ')')
            BOOST_THROW_EXCEPTION(ManagerException() << mstd::error_message("Android log device syntax: android_log(tag)"));
        device = AndroidLogDevice(value.substr(12, value.length() - 13));
    }
#endif
    else
        BOOST_THROW_EXCEPTION(ManagerException() << mstd::error_message("Unknown log device: " + value));
    
    return new LogDevice(name, device);
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

void Manager::setupDevice(const std::string & prop, const std::string & value, boost::mutex::scoped_lock & lock)
{
    SharedDevices newDevices(new Devices(*devices_));
    if(prop == "remove")
    {
        if(value == "*")
            Devices(1).swap(*newDevices);
        else {
            if(!mlog::setupDevice(*newDevices, value, EraseDevice(), lock))
                BOOST_THROW_EXCEPTION(ManagerException() << mstd::error_message("Unknown log device: " + value));
        }
    } else {
        LogDevicePtr newDevice(createDevice(prop, value));

        if(!mlog::setupDevice(*newDevices, prop, ChangeDevice(newDevice), lock))
        {
            newDevices->resize(std::max(static_cast<size_t>(1), newDevices->size()));
            (*newDevices)[0].push_back(newDevice);
        }
    }

    devices_ = newDevices;
}

void Manager::setListener(LogLevel level, const Listener & listener)
{
    boost::lock_guard<boost::mutex> lock(mutex_);
    listenerLevel_ = level;
    listener_ = listener;
}

void Manager::output(const char * logger, const Buffer & buf)
{
    if(realtime_)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);
        queue_.push_back(Queue::value_type(logger, buf));
        cond_.notify_one();
        if(!threadStarted_)
        {
            threadStarted_ = true;
            thread_ = boost::thread(&Manager::execute, this);
        }
    } else {
        SharedDevices devices;
        {
            boost::lock_guard<boost::mutex> lock(mutex_);
            devices = devices_;
        }
        Listener listener;
        bool listenerChecked = false;
        process(*devices, logger, buf, listener, listenerChecked);
    }
}

void Manager::process(Devices & devices, const char * logger, const Buffer & buf, Listener & listener, bool & listenerChecked)
{
    char * p = bufferData(buf);
    uint32_t group = *mstd::pointer_cast<uint32_t*>(p);
    p += sizeof(group);
    LogLevel level = *mstd::pointer_cast<LogLevel*>(p);
    p += sizeof(level);
    size_t len = *mstd::pointer_cast<size_t*>(p);
    p += sizeof(len);

    DeviceGroup & devs = devices[0];
    DeviceGroup::iterator i = devs.begin();
    DeviceGroup::iterator end = devs.end();
    for(; i != end; ++i)
    {
        LogDevice & device = **i;
        device.output(level, p, len);
    }

    if(group && group < devices.size())
    {
        DeviceGroup & devs = devices[group];
        DeviceGroup::iterator i = devs.begin();
        DeviceGroup::iterator end = devs.end();
        for(; i != end; ++i)
        {
            LogDevice & device = **i;
            device.output(level, p, len);
        }
    }

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

void Manager::execute()
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
                SharedDevices devices = devices_;
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
    
detail::LoggerImpl & Manager::registerLogger(const std::string & name)
{
    boost::mutex::scoped_lock lock(mutex_);
    return registerLogger(name, lock);
}

detail::LoggerImpl & Manager::registerLogger(const std::string & name, boost::mutex::scoped_lock & lock)
{
    Loggers::iterator i = loggers_.find(name);
    if(i == loggers_.end())
        i = loggers_.insert(Loggers::value_type(name, detail::LoggerImpl(rootLogger_.level()))).first;
    
    return const_cast<detail::LoggerImpl&>(i->second);
}

void Manager::setAppName(const char * value)
{
    Data::instance().appname(value);
}

Manager::Manager()
    : rootLogger_(llWarning),
      devices_(new Devices(1, DeviceGroup(1, LogDevicePtr(new LogDevice("console", CFileLogDevice(stdout)))))),
      threadStarted_(false), realtime_(false)
{
#if MLOG_USE_BUFFERS
    mstd::buffers::instance();
#endif
    levelName(llError);
}

Manager::~Manager()
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

MSTD_SINGLETON_IMPLEMENTATION(Manager);

}

#endif
