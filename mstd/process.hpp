#pragma once

#include <boost/filesystem/path.hpp>

namespace mstd {

boost::filesystem::wpath executable_path();
void execute_file(const boost::filesystem::wpath & path);
void execute_file(const boost::filesystem::wpath & path, const std::vector<std::wstring> & arguments);
void make_executable(const boost::filesystem::wpath & path, bool user = true, bool group = true, bool other = true);

}
