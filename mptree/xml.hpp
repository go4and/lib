#pragma once

namespace rapidxml {
    template<class Ch>
    class xml_node;
}

namespace mptree {

namespace xml {

void write_open_tag(std::streambuf * buf, const char * name, size_t len, bool newLine);
void write_close_tag(std::streambuf * buf, const char * name, size_t len);
void write_ident(std::streambuf * buf, size_t ident);

template<class Value>
void do_write_node(std::ostream & out, const char * name, const Value & value, size_t ident)
{
    std::streambuf * buf = out.rdbuf();
    write_ident(buf, ident);
    size_t len = strlen(name);
    write_open_tag(buf, name, len, false);
    write_value(buf, value);
    write_close_tag(buf, name, len);
}

const char * node_name(const rapidxml::xml_node<char> & node);
const char * node_value(const rapidxml::xml_node<char> & node);

class iterator {
public:
    iterator(const rapidxml::xml_node<char> * value)
        : value_(value)
    {
    }

    void next();
    const rapidxml::xml_node<char> & operator*() const { return *value_; }
    const rapidxml::xml_node<char> * value() const { return value_; }
private:
    const rapidxml::xml_node<char> * value_;
};

inline bool operator!=(const iterator & lhs, const iterator & rhs)
{
    return lhs.value() != rhs.value();
}

inline iterator & operator++(iterator & iter)
{
    iter.next();
    return iter;
}

iterator node_begin(const rapidxml::xml_node<char> & node);
iterator node_end(const rapidxml::xml_node<char> & node);

}

using xml::node_name;
using xml::node_value;
using xml::node_begin;
using xml::node_end;

}
