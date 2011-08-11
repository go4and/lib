#pragma once

#include <boost/version.hpp>

#if BOOST_VERSION >= 104100

#include <functional>
#include <string>

#include <boost/algorithm/string/predicate.hpp>

#include <boost/filesystem/path.hpp>

#include <boost/property_tree/ptree_fwd.hpp>

#include "config.hpp"

namespace mstd {

MSTD_DECL void read_xml(const boost::filesystem::wpath & path, boost::property_tree::wptree & out);
MSTD_DECL void read_xml(const std::wstring & path, boost::property_tree::wptree & out);
MSTD_DECL void read_xml(const wchar_t * path, boost::property_tree::wptree & out);
MSTD_DECL void read_xml(std::istream & inp, boost::property_tree::wptree & out);

MSTD_DECL void write_xml(const boost::filesystem::wpath & path, const boost::property_tree::wptree & inp);
MSTD_DECL void write_xml(const std::wstring & path, const boost::property_tree::wptree & inp);
MSTD_DECL void write_xml(const wchar_t * path, const boost::property_tree::wptree & inp);
MSTD_DECL void write_xml(std::ostream & out, const boost::property_tree::wptree & inp);

}

#endif
