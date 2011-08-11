#if !defined(_STLP_NO_IOSTREAMS)

#include "xml.hpp"

#if BOOST_VERSION >= 104100

#include <boost/filesystem/fstream.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "strings.hpp"

namespace mstd {

void convert(const boost::property_tree::ptree & in, boost::property_tree::wptree & out)
{
    out.data() = deutf8(in.data());
    for(boost::property_tree::ptree::const_iterator i = in.begin(), end = in.end(); i != end; ++i)
    {
        out.push_back(boost::property_tree::wptree::value_type(deutf8(i->first), boost::property_tree::wptree()));
        convert(i->second, out.back().second);
    }
}

void convert(const boost::property_tree::wptree & in, boost::property_tree::ptree & out)
{
    out.data() = utf8(in.data());
    for(boost::property_tree::wptree::const_iterator i = in.begin(), end = in.end(); i != end; ++i)
    {
        out.push_back(boost::property_tree::ptree::value_type(utf8(i->first), boost::property_tree::ptree()));
        convert(i->second, out.back().second);
    }
}

template<class Ch>
void cleanup(boost::property_tree::basic_ptree<std::basic_string<Ch>, std::basic_string<Ch> > & tree)
{
    bool hasElements = false;
    for(typename boost::property_tree::basic_ptree<std::basic_string<Ch>, std::basic_string<Ch> >::iterator i = tree.begin(), end = tree.end(); i != end; ++i)
    {
        if(!hasElements && i->first != boost::property_tree::xml_parser::xmlattr<Ch>() && i->first != boost::property_tree::xml_parser::xmltext<Ch>())
            hasElements = true;
        cleanup(i->second);
    }
    if(hasElements)
        tree.data().clear();
}

void read_xml(std::istream & inp, boost::property_tree::wptree & out)
{
    out.clear();
    boost::property_tree::ptree temp;
    boost::property_tree::read_xml(inp, temp);
    cleanup(temp);
    convert(temp, out);
}

void read_xml(const boost::filesystem::wpath & path, boost::property_tree::wptree & out)
{
    boost::filesystem::ifstream inp(path, std::ios::binary);
    read_xml(inp, out);
}

void read_xml(const std::wstring & path, boost::property_tree::wptree & out)
{
    read_xml(boost::filesystem::wpath(path), out);
}

void read_xml(const wchar_t * path, boost::property_tree::wptree & out)
{
    read_xml(boost::filesystem::wpath(path), out);
}

void write_xml(std::ostream & out, const boost::property_tree::wptree & inp)
{
    boost::property_tree::ptree temp;
    convert(inp, temp);
    boost::property_tree::write_xml(out, temp, boost::property_tree::xml_writer_make_settings(' ', 2));
}

void write_xml(const boost::filesystem::wpath & path, const boost::property_tree::wptree & inp)
{
    boost::filesystem::ofstream out(path);
    write_xml(out, inp);
}

void write_xml(const std::wstring & path, const boost::property_tree::wptree & inp)
{
    write_xml(boost::filesystem::wpath(path), inp);
}

void write_xml(const wchar_t * path, const boost::property_tree::wptree & inp)
{
    write_xml(boost::filesystem::wpath(path), inp);
}

}

#endif

#endif
