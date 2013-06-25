/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

#if !defined(MLOG_BUILDING) && !MLOG_NO_LOGGING
#include <boost/config.hpp>
#include <boost/scoped_ptr.hpp>

#include <mstd/cstdint.hpp>

#include <mstd/buffers.hpp>
#include <mstd/singleton.hpp>
#endif

#include "Defines.h"
#include "Config.h"

namespace mlog {

#if !MLOG_NO_LOGGING
    namespace detail {
        class MLOG_DECL OStreamPool : public mstd::singleton<OStreamPool> {
        public:
            ~OStreamPool();

            std::ostream * take();
            void release(std::ostream * stream);
        private:
            OStreamPool();

            class Impl;
            boost::scoped_ptr<Impl> impl_;

            MSTD_SINGLETON_DECLARATION(OStreamPool);
        };
    }

    class MLOG_DECL Output : public boost::noncopyable {
    public:
        explicit Output()
            : out_(detail::OStreamPool::instance().take())
        {
        }
        
        ~Output()
        {
            detail::OStreamPool::instance().release(out_);
        }

        std::ostream & out()
        {
            BOOST_ASSERT(out_);
            return *out_;
        }
        
        void send(uint32_t group, LogLevel level, const char * logger);
    private:
        std::ostream * out_;
    };

#endif

}
