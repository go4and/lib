#ifdef _MSC_VER
#pragma once
#endif

#include <curl/curl.h>

#include <yajl/yajl_parse.h>

#include <sys/stat.h>

#include <boost/function.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>

#include <boost/date_time/posix_time/ptime.hpp>

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include <boost/intrusive/set.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <boost/scope_exit.hpp>

#include <mstd/atomic.hpp>
#include <mstd/cstdint.hpp>
#include <mstd/exception.hpp>
#include <mstd/filesystem.hpp>
#include <mstd/handle_base.hpp>
#include <mstd/intrusive.hpp>
#include <mstd/itoa.hpp>
#include <mstd/pointer_cast.hpp>
#include <mstd/reference_counter.hpp>
#include <mstd/singleton.hpp>

#include <mlog/Dumper.h>
#include <mlog/Logging.h>

#define MNET_BUILDING
