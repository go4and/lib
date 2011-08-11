#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/optional.hpp>

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
