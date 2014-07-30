/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

namespace nexus {

typedef unsigned char PacketCode;

const PacketCode pcConnected = 1;
const PacketCode pcDisconnected = 2;
const PacketCode pcFailed = 3;

class PacketReader;

}
