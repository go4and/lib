#pragma once

#include "src/parsers.hpp"
#include "src/writers.hpp"
#include "xml.hpp"

namespace mptree {

class node;

template<class T>
inline typename boost::disable_if<boost::is_base_of<node, T>, void>::type
write_node(std::ostream & out, const char * name, const T & value, size_t ident)
{
    xml::do_write_node(out, name, value, ident);
}

template<class T>
inline void write_node(std::ostream & out, const char * name, const std::vector<T> & value, size_t ident)
{
    for(auto i = value.begin(), end = value.end(); i != end; ++i)
        write_node(out, name, *i, ident);
}

class unparsed;
typedef boost::intrusive_ptr<unparsed> unparsed_ptr;

class unparsed_child {
public:
    explicit unparsed_child(const rapidxml::xml_node<char> & node);

    inline const std::string & name() const { return name_; }
    inline const std::string & value() const { return value_; }
    inline size_t next() const { return next_; }
    inline size_t child() const { return child_; }

    inline void next(size_t value) { next_ = value; }
    inline void child(size_t value) { child_ = value; }
private:
    std::string name_;
    std::string value_;
    size_t next_;
    size_t child_;
};

class unparsed : public mstd::reference_counter<unparsed> {
public:
    unparsed()
        : tail_(0) {}

    void add(const rapidxml::xml_node<char> & node);
    void write(std::ostream & out, size_t ident) const;
private:
    size_t tail_;
    std::vector<unparsed_child> children_;
};


class node {
public:
    node()
        : parsed_(false)
    {
    }

    void write(std::ostream & out, const char * name, size_t ident = 0) const;
    bool parse(const rapidxml::xml_node<char> & node);
    virtual void complete();
private:
    virtual bool parse_child(const rapidxml::xml_node<char> & node) = 0;
    virtual void do_complete() = 0;
    virtual void write_children(std::ostream & out, size_t ident) const = 0;

    bool parsed_;
    unparsed_ptr unparsed_;
};

template<class T>
inline typename boost::enable_if<boost::is_base_of<node, T>, void>::type complete_value(T & out) { out.complete(); }

template<class T>
inline typename boost::disable_if<boost::is_base_of<node, T>, void>::type complete_value(T & out) {}

inline bool parse_node(node & out, const rapidxml::xml_node<char> & node) { return out.parse(node); }
inline void write_node(std::ostream & out, const char * name, const node & node, size_t ident) { node.write(out, name, ident); }

#define MPTREE_NODE_ITEM_STRING_2(elem) BOOST_PP_STRINGIZE(BOOST_PP_SEQ_ELEM(1, elem))
#define MPTREE_NODE_ITEM_STRING_3(elem) BOOST_PP_SEQ_ELEM(2, elem)

#define MPTREE_NODE_INVOKE_PARENT_0(a)
#define MPTREE_NODE_INVOKE_PARENT_1(a) BOOST_PP_APPLY(a)

#define MPTREE_NODE_ITEM_STRING(elem) \
    BOOST_PP_CAT(MPTREE_NODE_ITEM_STRING_, BOOST_PP_SEQ_SIZE(elem))(elem)

#define MPTREE_NODE_PARSE_CHILD_ITEM(r, data, elem) \
        if(!strcmp(name, MPTREE_NODE_ITEM_STRING(elem))) \
            return parse_node(this->BOOST_PP_SEQ_ELEM(1, elem), node); \
        /**/

#define MPTREE_NODE_PARSE_CHILD(parentInvoker, parent, seq) \
    bool parse_child(const rapidxml::xml_node<char> & node) \
    { \
        using mptree::node_name; \
        using mptree::parse_node; \
        parentInvoker((if(parent::parse_child(node)) return true;)); \
        const char * name = node_name(node); \
        BOOST_PP_SEQ_FOR_EACH(MPTREE_NODE_PARSE_CHILD_ITEM, ~, seq); \
        return false; \
    } \
    /**/

#define MPTREE_NODE_DO_COMPLETE_ITEM(r, data, elem) complete_value(BOOST_PP_SEQ_ELEM(1, elem));
#define MPTREE_NODE_DO_COMPLETE(parentInvoker, parent, seq) \
    void do_complete() \
    { \
        using mptree::complete_value; \
        parentInvoker((parent::do_complete();)) \
        BOOST_PP_SEQ_FOR_EACH(MPTREE_NODE_DO_COMPLETE_ITEM, ~, seq); \
    } \
    /**/

#define MPTREE_NODE_WRITE_CHILD(r, data, elem) \
    write_node(out, MPTREE_NODE_ITEM_STRING(elem), BOOST_PP_SEQ_ELEM(1, elem), ident);
#define MPTREE_NODE_WRITE_CHILDREN(parentInvoker, parent, seq) \
    void write_children(std::ostream & out, size_t ident) const \
    { \
        using mptree::write_node; \
        parentInvoker((parent::write_children(out, ident))); \
        BOOST_PP_SEQ_FOR_EACH(MPTREE_NODE_WRITE_CHILD, ~, seq); \
    } \
    /**/

#define MPTREE_NODE_MEMBER(r, data, elem) BOOST_PP_SEQ_ELEM(0, elem) BOOST_PP_SEQ_ELEM(1, elem);
#define MPTREE_NODE_MEMBERS(seq) BOOST_PP_SEQ_FOR_EACH(MPTREE_NODE_MEMBER, ~, seq)

#define MPTREE_NODE_CHILDREN_IMPL(parentInvoker, parent, seq) \
    public: \
        MPTREE_NODE_MEMBERS(seq) \
    protected: \
        MPTREE_NODE_PARSE_CHILD(parentInvoker, parent, seq) \
        MPTREE_NODE_WRITE_CHILDREN(parentInvoker, parent, seq) \
        MPTREE_NODE_DO_COMPLETE(parentInvoker, parent, seq) \
        /**/

#define MPTREE_NODE_CHILDREN(seq) MPTREE_NODE_CHILDREN_IMPL(MPTREE_NODE_INVOKE_PARENT_0, ~, seq)
#define MPTREE_NODE_CHILDREN_WITH_PARENT(parent, seq) MPTREE_NODE_CHILDREN_IMPL(MPTREE_NODE_INVOKE_PARENT_1, parent, seq)

}
