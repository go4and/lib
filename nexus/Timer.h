/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace nexus {

void schedule(const mstd::command_queue::command_type & command, const boost::posix_time::time_duration & delay);

}
