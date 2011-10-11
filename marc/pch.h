#ifdef _MSC_VER
#pragma once
#endif

#include <boost/config.hpp>

#include <boost/filesystem/operations.hpp>

#include <boost/scope_exit.hpp>

#if BOOST_WINDOWS
#include <boost/preprocessor/seq/for_each.hpp>
#endif

#include <zlib.h>

#include <archive.h>

#include <mstd/filesystem.hpp>

#include <mlog/Logging.h>
