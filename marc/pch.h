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
