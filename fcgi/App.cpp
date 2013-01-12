#include "pch.h"

#include "Server.h"

#include "App.h"

MLOG_DECLARE_LOGGER(app);

namespace fcgi {

namespace {
    class AssignStopped {
    public:
        explicit AssignStopped(mstd::atomic<bool> & stopped)
            : stopped_(stopped) {}

        void operator()(int) const
        {
            stopped_ = true;
        }
    private:
        mstd::atomic<bool> & stopped_;
    };
}

void consoleLoop()
{
    mstd::atomic<bool> stopped(false);
    nexus::signalAction(nexus::signalTerm, false, AssignStopped(stopped));

    while(!stopped)
    {
        std::cout << "> ";
        std::string line;
        std::getline(std::cin, line);
        boost::trim(line);
        if(line.empty())
            continue;
        else if(line == "stop")
            break;
        else if(boost::starts_with(line, "log "))
        {
            try {
                mlog::Manager::instance().setup(line.substr(4));
            } catch(const mlog::ManagerException & exc) {
                std::cerr << "Failed: " << exc.what() << std::endl;
            }
        } else {
            std::cerr << "Unknown control sequence: " << line << std::endl;
        }
    }
    std::cout << "Application done" << std::endl;
}

void setupLocale()
{
    std::locale rus("");
    std::locale glob = std::locale::global(std::locale());
    std::locale loc(glob, rus, std::locale::ctype);
    std::locale::global(loc);
}

class AppContext::Impl {
public:
    Impl(const std::string & name, int argc, char * argv[])
        : pid_(name + ".pid")
    {
        setupLocale();
        mlog::setup((boost::to_upper_copy(name) + "_LOGGING").c_str(), name.c_str());

        std::cout << name << " started" << std::endl;

        iotp_ = boost::in_place();

        if(argc > 1)
            dbopts_ = argv[1];

#if !defined(BOOST_WINDOWS)
        {
            boost::filesystem::ofstream out(pid_);
            out << getpid() << std::endl;
        }
#endif
    }

    void run(unsigned short port, const boost::function<void()> & starter, const RequestHandler & handler)
    {
        MLOG_MESSAGE(Debug, "run()");

        Server server(iotp_->ioService(), handler);

        starter();
        server.start(port);
        iotp_->start(12);

        consoleLoop();

        server.stop();

        iotp_->stop();
    }

    boost::asio::io_service & ioService()
    {
        return iotp_->ioService();
    }

    const std::string & dbopts()
    {
        return dbopts_;
    }

    ~Impl()
    {
        iotp_->stop();

        try {
            remove(pid_);
        } catch(boost::system::system_error &) {
        }
    }
private:
    boost::filesystem::path pid_;
    boost::optional<nexus::IoThreadPool> iotp_;
    std::string dbopts_;
};

AppContext::AppContext(const std::string & name, int argc, char * argv[])
    : impl_(new Impl(name, argc, argv))
{
}

AppContext::~AppContext()
{
}

void AppContext::run(unsigned short port, const boost::function<void()> & starter, const RequestHandler & handler)
{
    impl_->run(port, starter, handler);
}

boost::asio::io_service & AppContext::ioService()
{
    return impl_->ioService();
}

const std::string & AppContext::dbopts()
{
    return impl_->dbopts();
}

}
