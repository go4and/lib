#pragma once

#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace nexus {

void schedule(const mstd::command_queue::command_type & command, const boost::posix_time::time_duration & delay);

}
