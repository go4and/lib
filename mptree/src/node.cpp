#include "pch.hpp"

#include "node.hpp"

namespace mptree {

namespace {

void append(char *& out, const char * value, size_t len)
{
    memcpy(out, value, len);
    out += len;
}

template<class Ch>
bool check_escape(char *& out, Ch ch)
{
    if(ch == '&')
        append(out, "&amp;", 5);
    else if(ch == '"')
        append(out, "&quot;", 6);
    else if(ch == '\'')
        append(out, "&apos;", 6);
    else if(ch == '<')
        append(out, "&lt;", 4);
    else if(ch == '>')
        append(out, "&gt;", 4);
    else
        return false;
    return true;
}

template<class Out>
size_t fill_unparsed(Out & out, const rapidxml::xml_node<char> & node)
{
    size_t idx = out.size();
    out.push_back(unparsed_child(node));
    size_t tail = 0;
    for(auto i = xml::node_begin(node), end = xml::node_end(node); i != end; ++i)
    {
        size_t child = fill_unparsed(out, *i);
        if(tail)
            out[tail].next(child);
        else
            out[idx].child(child);
        tail = child;
    }
    return idx;
}

}

unparsed_child::unparsed_child(const rapidxml::xml_node<char> & node)
    : name_(xml::node_name(node)), value_(xml::node_value(node)), next_(0), child_(0)
{
}

template<class Col>
void write_unparsed(const Col & col, size_t idx, std::streambuf * out, size_t ident)
{
    const unparsed_child & child = col[idx];
    xml::write_ident(out, ident);
    if(child.child())
    {
        xml::write_open_tag(out, child.name().c_str(), child.name().length(), true);
        write_unparsed(col, child.child(), out, ident + 1);
        xml::write_ident(out, ident);
        xml::write_close_tag(out, child.name().c_str(), child.name().length());
    } else if(!child.value().empty()) {
        xml::write_open_tag(out, child.name().c_str(), child.name().length(), false);
        char buffer[0x40];
        char * o = buffer;
        char * limit = buffer + sizeof(buffer) - 6;
        for(std::string::const_iterator p = child.value().begin(), end = child.value().end(); p != end; ++p)
        {
            char ch = *p;
            if(!check_escape(o, ch))
                *o++ = ch;
            if(o >= limit)
            {
                out->sputn(buffer, o - buffer);
                o = buffer;
            }
        }
        if(o != buffer)
            out->sputn(buffer, o - buffer);
        xml::write_close_tag(out, child.name().c_str(), child.name().length());
    } else {
        out->sputc('<');
        out->sputn(child.name().c_str(), child.name().length());
        out->sputn("/>\n", 3);
    }
    if(child.next())
        write_unparsed(col, child.next(), out, ident);
}

void unparsed::add(const rapidxml::xml_node<char> & node)
{
    size_t idx = fill_unparsed(children_, node);
    if(tail_ != idx)
        children_[tail_].next(idx);
    tail_ = idx;
}

void unparsed::write(std::ostream & out, size_t ident) const
{
    std::streambuf * buf = out.rdbuf();
    write_unparsed(children_, 0, buf, ident);
}

bool node::parse(const rapidxml::xml_node<char> & node)
{
    if(parsed_)
    {
        return false;
    } else {
        for(auto i = xml::node_begin(node), end = xml::node_end(node); i != end; ++i)
        {
            if(!parse_child(*i))
            {
                if(!unparsed_)
                    unparsed_.reset(new unparsed);
                unparsed_->add(*i);
            }
        }
        complete();
        parsed_ = true;
        return true;
    }
}

void node::write(std::ostream & out, const char * name, size_t ident) const
{
    std::streambuf * buf = out.rdbuf();
    xml::write_ident(buf, ident);
    size_t len = strlen(name);
    xml::write_open_tag(buf, name, len, true);
    write_children(out, ident + 1);
    if(unparsed_)
        unparsed_->write(out, ident + 1);
    xml::write_ident(buf, ident);
    xml::write_close_tag(buf, name, len);
}

void node::complete()
{
    if(!parsed_)
        do_complete();
}

void write_value(std::streambuf * buf, const std::wstring & value)
{
    char buffer[0x40];
    char * out = buffer;
    char * limit = buffer + sizeof(buffer) - 6;
    const wchar_t ones = static_cast<wchar_t>(-1);
    for(std::wstring::const_iterator p = value.begin(), end = value.end(); p != end; ++p)
    {
        wchar_t ch = *p;
        if(!check_escape(out, ch))
        {
            if (!(ch & (ones ^ 0x7f))) {
                *out = static_cast<char>(ch);
                ++out;
            } else if (ch & (ones ^ 0x07ff)) {
                *out = static_cast<char>(0xe0 | ((ch >> 12) & 0x0f));
                *++out = static_cast<char>(0x80 | ((ch >> 6) & 0x3f));
                *++out = static_cast<char>(0x80 | (ch & 0x3f));
                ++out;
            } else {
                *out = static_cast<char>(0xc0 | ((ch >> 6) & 0x1f));
                *++out = static_cast<char>(0x80 | (ch & 0x3f));
                ++out;
            }
        }
        if(out >= limit)
        {
            buf->sputn(buffer, out - buffer);
            out = buffer;
        }
    }
    if(out != buffer)
        buf->sputn(buffer, out - buffer);
}

void write_value(std::ostream & out, const char * name, double value, size_t ident)
{
    std::streambuf * buf = out.rdbuf();
    xml::write_ident(buf, ident);
    size_t len = strlen(name);
    xml::write_open_tag(buf, name, len, false);
    out << value;
    xml::write_close_tag(buf, name, len);
}

}

