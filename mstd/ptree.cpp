/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include "strings.hpp"

#include "ptree.hpp"

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

void dump_tree(std::ostream & out, const boost::property_tree::ptree & tree, int ident)
{
    if(!tree.data().empty())
        out << ": '" << tree.data() << "'";
    out << std::endl;
    for(boost::property_tree::ptree::const_iterator i = tree.begin(), end = tree.end(); i != end; ++i)
    {
        for(int j = 0; j != ident; ++j)
            out << ' ';
        out << '"' << i->first << '"';
        dump_tree(out, i->second, ident + 2);
    }
}

void dump_tree(std::ostream & out, const boost::property_tree::wptree & tree, int ident)
{
    if(!tree.data().empty())
        out << ": '" << utf8(tree.data()) << "'";
    out << std::endl;
    for(boost::property_tree::wptree::const_iterator i = tree.begin(), end = tree.end(); i != end; ++i)
    {
        for(int j = 0; j != ident; ++j)
            out << ' ';
        out << '"' << utf8(i->first) << '"';
        dump_tree(out, i->second, ident + 2);
    }
}

}
