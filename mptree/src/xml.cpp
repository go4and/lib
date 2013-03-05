#include "pch.hpp"

#include "../rapidxml/rapidxml.hpp"

#include "node.hpp"

#include "xml.hpp"

namespace mptree {

namespace {

void parse(const parser_state & state, rapidxml::xml_node<> & input)
{
    for(auto i = input.first_node(); i; i = i->next_sibling())
        if(i->type() == rapidxml::node_element)
        {
            parser_state ps = state.child_parser(i->name(), state.data);
            if(ps.data)
            {
                if(ps.parse_value)
                    ps.parse_value(i->value(), ps.data);
                if(ps.child_parser)
                    parse(ps, *i);
            }
        }
    state.child_parser(0, state.data);
}

void parse(node & output, rapidxml::xml_node<> & input)
{
    parser_state state = make_parser(output);
    parse(state, input);
}

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

class xml_writer : public node_writer {
public:
    xml_writer(std::streambuf * buf)
        : buf_(buf), ident_(0)
    {
    }

    void begin_struct(const char * name, bool)
    {
        write_ident(buf_, ident_);
        write_open_tag(buf_, name, strlen(name), true);
        ++ident_;
    }
    
    void end_struct(const char * name, bool)
    {
        --ident_;
        write_ident(buf_, ident_);
        write_close_tag(buf_, name, strlen(name));
    }

    void begin_value(const char * name, bool)
    {
        write_ident(buf_, ident_);
        write_open_tag(buf_, name, strlen(name), false);
    }

    void end_value(const char * name, bool)
    {
        write_close_tag(buf_, name, strlen(name));
    }
    
    void write_raw(const char * buffer, size_t len)
    {
        buf_->sputn(buffer, len);
    }

    void begin_array(const char * name)
    {
    }

    void end_array(const char * name)
    {
    }
    
    void empty_node(const char * name)
    {
        write_ident(buf_, ident_);
        buf_->sputc('<');
        buf_->sputn(name, strlen(name));
        buf_->sputn("/>\n", 3);
    }
    
    void write_escaped(const char * p, size_t len)
    {
        char buffer[0x40];
        char * o = buffer;
        char * limit = buffer + sizeof(buffer) - 6;
        for(const char * end = p + len; p != end; ++p)
        {
            char ch = *p;
            if(!check_escape(o, ch))
                *o++ = ch;
            if(o >= limit)
            {
                buf_->sputn(buffer, o - buffer);
                o = buffer;
            }
        }
        if(o != buffer)
            buf_->sputn(buffer, o - buffer);
    }

    void write_escaped(const wchar_t * p, size_t len)
    {
        char buffer[0x40];
        char * out = buffer;
        char * limit = buffer + sizeof(buffer) - 6;
        const wchar_t ones = static_cast<wchar_t>(-1);
        for(const wchar_t * end = p + len; p != end; ++p)
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
                buf_->sputn(buffer, out - buffer);
                out = buffer;
            }
        }
        if(out != buffer)
            buf_->sputn(buffer, out - buffer);
    }
private:
    std::streambuf * buf_;
    size_t ident_;
};

}

void parse_xml(node & root, char * data, const char * root_name)
{
    rapidxml::xml_document<> document;
    document.parse<0>(data);
    auto child = document.first_node();
    if(child)
        parse(root, *child);
    else
        root.complete();
}

void write_xml(std::ostream & out, const node & root, const char * root_name)
{
    xml_writer writer(out.rdbuf());
    root.write(writer, root_name, false);
}

}
