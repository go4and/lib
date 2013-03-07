#pragma once

namespace mptree {

struct subnet {
    boost::asio::ip::address_v4 address;
    uint32_t mask;
};

}
