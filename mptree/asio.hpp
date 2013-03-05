#pragma once

namespace mptree {

bool parse_value(boost::asio::ip::address & out, const char * val)
{
    out = boost::asio::ip::address::from_string(val);
}

}
