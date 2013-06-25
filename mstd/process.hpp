/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

#include <boost/filesystem/path.hpp>

namespace mstd {

boost::filesystem::path executable_path();
void execute_file(const boost::filesystem::wpath & path);
void execute_file(const boost::filesystem::wpath & path, const std::vector<std::wstring> & arguments);
void make_executable(const boost::filesystem::wpath & path, bool user = true, bool group = true, bool other = true);

}
