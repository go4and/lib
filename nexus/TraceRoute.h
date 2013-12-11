#pragma once

#ifndef NEXUS_BUILDING
#include <string>

#include <boost/asio/io_service.hpp>
#include <boost/function.hpp>
#endif

namespace nexus {

std::string makeTrace(const std::string & host);

void makeTraceAsync(boost::asio::io_service & ios, const std::string & host, const boost::function<void(const std::string &)> & listener);

}
