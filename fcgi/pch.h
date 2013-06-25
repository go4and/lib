/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/optional.hpp>
#include <boost/unordered_map.hpp>

#include <boost/algorithm/string.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include <boost/utility/in_place_factory.hpp>

#include <mstd/cstdint.hpp>
#include <mstd/hton.hpp>
#include <mstd/reference_counter.hpp>

#include <mlog/Dumper.h>
#include <mlog/Logging.h>
#include <mlog/Utils.h>

#include <nexus/Buffer.h>
#include <nexus/Handler.h>
#include <nexus/IoThreadPool.h>
#include <nexus/PacketReader.h>
#include <nexus/Signals.h>
#include <nexus/Socket.h>
#include <nexus/Utils.h>
