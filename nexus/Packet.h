/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
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
