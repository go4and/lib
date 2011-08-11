#pragma once

#ifndef NEXUS_BUILDING

#include <mstd/cstdint.hpp>

#endif

namespace nexus {

typedef boost::uint8_t PacketCode;

const PacketCode pcConnected = 1;
const PacketCode pcDisconnected = 2;

class PacketReader;

}
