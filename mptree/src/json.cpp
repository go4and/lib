/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
include "src/pch.hpp"

#include "../node.hpp"
#include "write_helper.hpp"

#include "../json.hpp"

namespace mptree {

namespace {

class json_escape {
public:
    template<class Ch>
    bool operator()(char *& out, Ch ch) const
    {
        switch (ch) {
        case '\r': append(out, "\\r", 2); break;
        case '\n': append(out, "\\n", 2); break;
        case '\\': append(out, "\\\\", 2); break;
        case '"': append(out, "\\\"", 2); break;
        case '\f': append(out, "\\f", 2); break;
        case '\b': append(out, "\\b", 2); break;
        case '\t': append(out, "\\t", 2); break;
        default:
            if (static_cast<unsigned char>(ch) < 32)
            {
                char buf[] = "\\u0000";
                buf[4] = mstd::hex_table[static_cast<unsigned char>(ch) >> 4];
                buf[5] = mstd::hex_table[ch & 0x0f];
                append(out, buf, 6);
            } else
                return false;
            break;
        }
        return true;
    }
};

class json_writer : public node_writer {
public:
    explicit json_writer(std::streambuf * buf)
        : buf_(buf)
    {
        first_.push_back(1);
    }

    void begin_struct(const char * name, bool in_array)
    {
        process_ident();

        if(name && !in_array)
        {
            buf_->sputc('"');
            buf_->sputn(name, strlen(name));
            buf_->sputn("\": {", 4);
        } else {
            buf_->sputc('{');
        }
        first_.push_back(1);
    }
    
    void end_struct(const char * name, bool in_array)
    {
        dec_ident();
        buf_->sputc('}');
    }

    void begin_value(const char * name, bool in_array)
    {
        process_ident();

        if(!in_array)
        {
            buf_->sputc('"');
            buf_->sputn(name, strlen(name));
            buf_->sputn("\": \"", 4);
        } else
            buf_->sputc('"');
    }
    
    void end_value(const char * name, bool in_array)
    {
        buf_->sputc('"');
    }

    void begin_array(const char * name)
    {
        process_ident();

        buf_->sputc('"');
        buf_->sputn(name, strlen(name));
        buf_->sputn("\": [", 4);

        first_.push_back(1);
    }
    
    void end_array(const char * name)
    {
        dec_ident();
        buf_->sputc(']');
    }

    void empty_node(const char * name)
    {
        begin_value(name, false);
        end_value(name, false);
    }

    void write_raw(const char * buffer, size_t len)
    {
        buf_->sputn(buffer, len);
    }

    void write_escaped(const wchar_t * value, size_t len)
    {
        mptree::write_escaped(buf_, value, len, json_escape());
    }
    
    void write_escaped(const char * value, size_t len)
    {
        mptree::write_escaped(buf_, value, len, json_escape());
    }
private:
    inline void process_ident()
    {
        if(first_.back())
        {
            buf_->sputc('\n');
            first_.back() = 0;
        } else
            buf_->sputn(",\n", 2);
        write_ident(buf_, first_.size() - 1);
    }
    
    inline void dec_ident()
    {
        bool empty = first_.back();
        first_.pop_back();
        if(!empty)
        {
            buf_->sputc('\n');
            write_ident(buf_, first_.size() - 1);
        }
    }

    std::streambuf * buf_;
    std::vector<char> first_;
};

class json_builder {
public:
    explicit json_builder(node & output)
        : output_(output)
    {
    }

    static int value_number(void * ctx, const char * numberVal, size_t numberLen)
    {
        return value_string(ctx, mstd::pointer_cast<const unsigned char*>(numberVal), numberLen);
    }

    static int value_string(void * ctx, const unsigned char * value, size_t len)
    {
        json_builder * self = static_cast<json_builder*>(ctx);

        auto & back = self->states_.back();
        if(back.parser.child_parser)
        {
            parser_state temp = back.parser.child_parser(back.name, back.nameLen, back.parser.data);

            if(temp.parse_value)
                temp.parse_value(mstd::pointer_cast<const char*>(value), len, temp.data);
        }
        return 1;
    }

    static int child_start(void * ctx)
    {
        json_builder * self = static_cast<json_builder*>(ctx);

        if(!self->states_.empty())
        {
            auto & back = self->states_.back();
            parser_state & last = back.parser;
            if(last.child_parser)
                self->states_.push_back(state(last.child_parser(back.name, back.nameLen, last.data)));
        } else {
            self->states_.push_back(state(make_parser(self->output_)));
        }

        return 1;
    }

    static int child_name(void * ctx, const unsigned char * name, size_t len)
    {
        json_builder * self = static_cast<json_builder*>(ctx);

        auto & back = self->states_.back();
        back.name = mstd::pointer_cast<const char*>(name);
        back.nameLen = len;
        return 1;
    }
    
    static int child_end(void * ctx)
    {
        json_builder * self = static_cast<json_builder*>(ctx);

        parser_state & last = self->states_.back().parser;
        if(last.child_parser)
            last.child_parser(0, 0, last.data);
        self->states_.pop_back();
        return 1;
    }
private:
    struct state {
        parser_state parser;
        const char * name;
        size_t nameLen;
 
        explicit state(const parser_state & p)
            : parser(p), name(0), nameLen(0)
        {
        }
    };

    node & output_;
    std::vector<state> states_;
};

yajl_callbacks json_callbacks = {
    NULL,
    NULL,
    NULL,
    NULL,
    &json_builder::value_number,
    &json_builder::value_string,
    &json_builder::child_start,
    &json_builder::child_name,
    &json_builder::child_end,
    NULL,
    NULL,
};

}

void parse_json(node & root, const char * data, size_t len)
{
    json_builder builder(root);
    mstd::handle_base<yajl_handle, mstd::global_fun_traits<yajl_handle, void, &yajl_free> > yajl(yajl_alloc(&json_callbacks, NULL, &builder));
    const unsigned char * buf = mstd::pointer_cast<const unsigned char*>(data);
    yajl_status res = yajl_parse(*yajl, buf, len);

    if(res == yajl_status_error)
    {
        unsigned char * err = yajl_get_error(*yajl, 1, buf, len);
        const char * error = mstd::pointer_cast<const char*>(err);
        (void)error;
        BOOST_ASSERT(false);
        yajl_free_error(*yajl, err);
    }
}

void write_json(std::ostream & out, const node & root)
{
    json_writer writer(out.rdbuf());
    root.write(writer, 0, false);
}

}
