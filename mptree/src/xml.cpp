#include "pch.hpp"

#include "../rapidxml/rapidxml.hpp"

#include "xml.hpp"

namespace mptree { namespace xml {

void write_open_tag(std::streambuf * buf, const char * name, size_t len, bool newLine)
{
    buf->sputc('<');
    buf->sputn(name, len);
    if(newLine)
        buf->sputn(">\n", 2);
    else
        buf->sputc('>');
}

void write_close_tag(std::streambuf * buf, const char * name, size_t len)
{
    buf->sputn("</", 2);
    buf->sputn(name, len);
    buf->sputn(">\n", 2);
}

void write_ident(std::streambuf * buf, size_t ident)
{
    for(size_t i = 0; i != ident; ++i)
        buf->sputn("  ", 2);
}

const char * node_name(const rapidxml::xml_node<char> & node)
{
    return node.name();
}

const char * node_value(const rapidxml::xml_node<char> & node)
{
    return node.value();
}

namespace {

inline const rapidxml::xml_node<char> * filter(const rapidxml::xml_node<char> * value)
{
    while(value && value->type() != rapidxml::node_element)
        value = value->next_sibling();
    return value;
}

}

void iterator::next()
{
    value_ = filter(value_->next_sibling());
}

iterator node_begin(const rapidxml::xml_node<char> & node)
{
    return iterator(filter(node.first_node()));
}

iterator node_end(const rapidxml::xml_node<char> & node)
{
    return iterator(0);
}

} }
