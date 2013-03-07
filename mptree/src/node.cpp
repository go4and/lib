#include "pch.hpp"

#include "node.hpp"

namespace mptree {


template<class Col>
void write_unparsed(node_writer & writer, const Col & col, size_t idx, bool in_array)
{
    const auto & self = col[idx];
    bool nextSame = self.next() && col[self.next()].name() == self.name();
    if(nextSame && !in_array)
    {
        writer.begin_array(self.name().c_str());
        in_array = true;
    }
    if(self.child())
    {
        writer.begin_struct(self.name().c_str(), in_array);
        write_unparsed(writer, col, self.child(), false);
        writer.end_struct(self.name().c_str(), in_array);
    } else if(!self.value().empty())
    {
        writer.begin_value(self.name().c_str(), in_array);
        writer.write_escaped(self.value().c_str(), self.value().length());
        writer.end_value(self.name().c_str(), in_array);
    } else {
        writer.empty_node(self.name().c_str());
    }
    if(!nextSame && in_array)
        writer.end_array(self.name().c_str());
    if(self.next())
        write_unparsed(writer, col, self.next(), nextSame);
}

class unparsed_child {
public:
    explicit unparsed_child(unparsed & holder, const char * name, size_t len)
        : holder_(holder), name_(name, len), next_(0), child_(0), tail_(0)
    {
    }

    unparsed & holder() const { return holder_; }

    inline const std::string & name() const { return name_; }
    inline const std::string & value() const { return value_; }
    inline size_t next() const { return next_; }
    inline size_t child() const { return child_; }

    inline void next(size_t value) { next_ = value; }
    inline void child(size_t value) { child_ = value; }
private:
    unparsed & holder_;
    std::string name_;
    std::string value_;
    size_t next_;
    size_t child_;
    size_t tail_;
    
    static parser_state unparsed_child_parser(const char * name, size_t len, void * data);

    static bool unparsed_parse_value(const char * value, size_t len, void * data)
    {
        unparsed_child * self = static_cast<unparsed_child*>(data);
        self->value_.assign(value, len);
        return true;
    }

    friend class unparsed;
};

class unparsed : public mstd::reference_counter<unparsed> {
public:
    unparsed()
        : tail_(0) {}

    parser_state add(const char * name, size_t len)
    {
        size_t idx = children_.size();
        children_.push_back(unparsed_child(*this, name, len));
        if(tail_ != idx)
            children_[tail_].next(idx);
        tail_ = idx;
        return parser_state(&unparsed_child::unparsed_child_parser, &unparsed_child::unparsed_parse_value, &children_[idx]);
    }

    void write(node_writer & writer) const
    {
        write_unparsed(writer, children_, 0, false);
    }
private:
    size_t tail_;
    boost::container::stable_vector<unparsed_child> children_;
    
    friend class unparsed_child;
};

parser_state unparsed_child::unparsed_child_parser(const char * name, size_t len, void * data)
{
    if(!name)
        return parser_state(0, 0, 0);
    unparsed_child * self = static_cast<unparsed_child*>(data);
    auto & children = self->holder_.children_;
    size_t idx = children.size();
    children.push_back(unparsed_child(self->holder_, name, len));
    if(self->tail_)
        children[self->tail_].next_ = idx;
    else
        self->child_ = idx;
    self->tail_ = idx;
    return parser_state(&unparsed_child_parser, &unparsed_parse_value, &children[idx]);
    
}

void node::complete()
{
    if(!parsed_)
        do_complete();
}

void node::write(node_writer & writer, const char * name, bool in_array) const
{
    writer.begin_struct(name, in_array);
    write_children(writer);
    if(unparsed_)
        unparsed_->write(writer);
    writer.end_struct(name, in_array);
}

parser_state make_unparsed_parser(const char * name, size_t len, unparsed_ptr & out)
{
    if(!out)
        out.reset(new unparsed());
    return out->add(name, len);
}

void intrusive_ptr_add_ref(unparsed * obj)
{
    mstd::reference_counter<unparsed> * counter = obj;
    intrusive_ptr_add_ref(counter);
}

void intrusive_ptr_release(unparsed * obj)
{
    mstd::reference_counter<unparsed> * counter = obj;
    intrusive_ptr_release(counter);
}

}

