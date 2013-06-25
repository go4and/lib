/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

namespace mlog {

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

}
