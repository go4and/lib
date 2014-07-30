/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#ifdef _MSC_VER
#pragma once
#endif

#include <curl/curl.h>

#include <yajl/yajl_parse.h>

#include <openssl/ssl.h>

#include <sys/stat.h>

#include <unordered_map>

#include <boost/intrusive_ptr.hpp>
#include <boost/scope_exit.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>

#include <boost/date_time/posix_time/ptime.hpp>

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include <boost/intrusive/set.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <boost/variant/get.hpp>
#include <boost/variant/variant.hpp>

#include <mstd/atomic.hpp>
#include <mstd/cstdint.hpp>
#include <mstd/enum_utils.hpp>
#include <mstd/exception.hpp>
#include <mstd/filesystem.hpp>
#include <mstd/handle_base.hpp>
#include <mstd/intrusive.hpp>
#include <mstd/itoa.hpp>
#include <mstd/pointer_cast.hpp>
#include <mstd/rc_buffer.hpp>
#include <mstd/singleton.hpp>

#include <mlog/Dumper.h>
#include <mlog/Logging.h>

#define MNET_BUILDING
