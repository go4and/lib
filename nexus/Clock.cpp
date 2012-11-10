#include "pch.h"

#include "Clock.h"

MLOG_DECLARE_LOGGER(clock);

namespace nexus {

namespace {

mstd::atomic<Milliseconds> out;

void tickThread()
{
    try {
        boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
        boost::posix_time::ptime timeStart = Clock::timeStart();
        boost::posix_time::time_duration step = boost::posix_time::milliseconds(10);
        size_t i = 1;
        while(!boost::this_thread::interruption_requested())
        {
            boost::this_thread::sleep(now);
            out = (now - timeStart).total_milliseconds();
            if(!--i)
            {
                boost::posix_time::ptime cur = boost::posix_time::microsec_clock::universal_time();
                if((cur - now).total_milliseconds() > 50)
                    MLOG_MESSAGE(Warning, "Long delay: " << (cur - now).total_milliseconds());
                now = cur;
                i = 100;
            }

            now += step;
        }
    } catch(boost::thread_interrupted&) {
    }
}

class Ticker {
public:
    ~Ticker()
    {
        thread_.interrupt();
        thread_.join();
    }

    bool dummy() const
    {
        return thread_.joinable();
    }

    void start()
    {
        boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
        out = (now - boost::posix_time::ptime(boost::gregorian::date(1970, boost::date_time::Jan, 1))).total_milliseconds();

        thread_ = boost::thread(&tickThread);
    }

    Ticker()
    {
    }
private:
    boost::thread thread_;
};

Ticker ticker;

}

Milliseconds Clock::milliseconds()
{
    return out;
}

const boost::posix_time::ptime & Clock::timeStart()
{
    static const boost::posix_time::ptime result(boost::gregorian::date(1970, boost::date_time::Jan, 1));
    return result;
}

void Clock::start()
{
    ticker.start();
}

}
