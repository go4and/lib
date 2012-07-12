#include "pch.h"

#if !MLOG_NO_LOGGING

#include "Logger.h"

#include "Utils.h"

namespace mlog {

namespace {

struct LevelData {
    std::string     name;
    std::string     escape;
    boost::uint16_t color;
};

class InitLogNames : public mstd::singleton<InitLogNames> {
public:
    const LevelData & data(mlog::LogLevel level) const
    {
        return levels_[level];
    }
private:
    InitLogNames()
    {
        levels_.reserve(8);
        add("EMG",  "\033[35;1m", 0x0d);
        add("ALR",  "\033[35;1m", 0x0d);
        add("CRT",  "\033[35;1m", 0x0d);
        add("ERR", "\033[31;22m", 0x01);
        add("WRN",  "\033[33;1m", 0x0b);
        add("NOT", "\033[37;22m", 0x0f);
        add("INF",            "", 0x07);
        add("DBG",            "", 0x07);
    }

    void add(const std::string & name, const std::string & escape, boost::uint16_t color)
    {
        LevelData data = {name, escape, color};
        levels_.push_back(data);
    }

    std::vector<LevelData> levels_;

    MSTD_SINGLETON_DECLARATION(InitLogNames);
};

void putString(std::streambuf * buf, const std::string & str)
{
    buf->sputn(str.c_str(), str.length());
}

#define PUT_ARRAY(t) buf->sputn(t, sizeof(t) - 1)

#if MLOG_USE_MARKUP
#  define PUT_MARKUP_ARRAY(t) PUT_ARRAY(t)
#else
#  define PUT_MARKUP_ARRAY(t)
#endif

}

void Logger::outputHeader(std::ostream & out, mlog::LogLevel level)
{
    const InitLogNames & names = InitLogNames::instance();
    const LevelData & data = names.data(level);
    
    std::streambuf * buf = out.rdbuf();
#if MLOG_USE_MARKUP
    putString(buf, data.escape);
#endif

    putString(buf, data.name);

    PUT_MARKUP_ARRAY("\033[0m");

    PUT_ARRAY(" [");
    PUT_MARKUP_ARRAY("\033[36m");
    mstd::thread_id tid = mstd::this_thread_id();
    out << tid;
    PUT_MARKUP_ARRAY("\033[0m");

    PUT_ARRAY("] ");

    PUT_MARKUP_ARRAY("\033[34;1m");
    out << mlog::onow();
    PUT_MARKUP_ARRAY("\033[0m");

    PUT_ARRAY(" <");
    PUT_MARKUP_ARRAY("\033[32;1m");
    putString(buf, name_);
    PUT_MARKUP_ARRAY("\033[0m");
    PUT_ARRAY("> ");

#if MLOG_USE_MARKUP
    out << data.escape;
#endif
    // TODO direct streambuf
}

const std::string & levelName(LogLevel level)
{
    const InitLogNames & names = InitLogNames::instance();
    return names.data(level).name;
}

}

#endif
