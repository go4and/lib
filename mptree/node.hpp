#pragma once

#include "src/parsers.hpp"
#include "src/writers.hpp"

namespace mptree {

class node;

class unparsed;
typedef boost::intrusive_ptr<unparsed> unparsed_ptr;
void intrusive_ptr_add_ref(unparsed * obj);
void intrusive_ptr_release(unparsed * obj);

class node {
public:
    node()
        : parsed_(false)
    {
    }

    void mark_parsed()
    {
        parsed_ = true;
    }

    static parser_state static_child_parser(const char * name, void * data)
    {
        return static_cast<node*>(data)->child_parser(name, true);
    }

    virtual void complete();
    virtual void write(node_writer & writer, const char * name, bool in_array) const;
private:
    virtual parser_state child_parser(const char * name, bool final) = 0;
    virtual void do_complete() = 0;
    virtual void write_children(node_writer & writer) const = 0;

    bool parsed_;
protected:
    unparsed_ptr unparsed_;
};

template<class T>
inline typename boost::enable_if<boost::is_base_of<node, T>, void>::type complete_value(T & out) { out.complete(); }

template<class T>
inline typename boost::disable_if<boost::is_base_of<node, T>, void>::type complete_value(T & out) {}

inline parser_state make_parser(node & out) { out.mark_parsed(); return parser_state(&node::static_child_parser, 0, &out); }
inline void write_node(node_writer & writer, const node & out, const char * name, bool in_array) { out.write(writer, name, in_array); }

parser_state make_unparsed_parser(const char * name, unparsed_ptr & unparsed);

#define MPTREE_NODE_ITEM_STRING_2(elem) BOOST_PP_STRINGIZE(BOOST_PP_SEQ_ELEM(1, elem))
#define MPTREE_NODE_ITEM_STRING_3(elem) BOOST_PP_SEQ_ELEM(2, elem)

#define MPTREE_NODE_INVOKE_PARENT_0(a)
#define MPTREE_NODE_INVOKE_PARENT_1(a) BOOST_PP_APPLY(a)

#define MPTREE_NODE_ITEM_STRING(elem) \
    BOOST_PP_CAT(MPTREE_NODE_ITEM_STRING_, BOOST_PP_SEQ_SIZE(elem))(elem)

#define MPTREE_NODE_PARSE_CHILD_ITEM(r, data, elem) \
        if(!strcmp(name, MPTREE_NODE_ITEM_STRING(elem))) \
            return make_parser(this->BOOST_PP_SEQ_ELEM(1, elem)); \
        /**/

#define MPTREE_NODE_PARSE(parentInvoker, parent, seq) \
    mptree::parser_state child_parser(const char * name, bool final) \
    { \
        using mptree::make_parser; \
        using mptree::make_unparsed_parser; \
        if(!name) { complete(); return mptree::parser_state(0, 0, 0); } \
        parentInvoker((mptree::parser_state temp = parent::child_parser(name, false); if(temp.data) return temp;)); \
        BOOST_PP_SEQ_FOR_EACH(MPTREE_NODE_PARSE_CHILD_ITEM, ~, seq); \
        if(!final) \
            return mptree::parser_state(0, 0, 0); \
        else \
            return make_unparsed_parser(name, unparsed_); \
    } \
    void parse_value(const char * value, void * data) \
    { \
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
    write_node(writer, BOOST_PP_SEQ_ELEM(1, elem), MPTREE_NODE_ITEM_STRING(elem), false);
#define MPTREE_NODE_WRITE_CHILDREN(parentInvoker, parent, seq) \
    void write_children(mptree::node_writer & writer) const \
    { \
        using mptree::write_node; \
        parentInvoker((parent::write_children(writer))); \
        BOOST_PP_SEQ_FOR_EACH(MPTREE_NODE_WRITE_CHILD, ~, seq); \
    } \
    /**/

#define MPTREE_NODE_MEMBER(r, data, elem) BOOST_PP_SEQ_ELEM(0, elem) BOOST_PP_SEQ_ELEM(1, elem);
#define MPTREE_NODE_MEMBERS(seq) BOOST_PP_SEQ_FOR_EACH(MPTREE_NODE_MEMBER, ~, seq)

#define MPTREE_NODE_CHILDREN_IMPL(parentInvoker, parent, seq) \
    public: \
        MPTREE_NODE_MEMBERS(seq) \
    protected: \
        MPTREE_NODE_PARSE(parentInvoker, parent, seq) \
        MPTREE_NODE_WRITE_CHILDREN(parentInvoker, parent, seq) \
        MPTREE_NODE_DO_COMPLETE(parentInvoker, parent, seq) \
        /**/

#define MPTREE_NODE_CHILDREN(seq) MPTREE_NODE_CHILDREN_IMPL(MPTREE_NODE_INVOKE_PARENT_0, ~, seq)
#define MPTREE_NODE_CHILDREN_WITH_PARENT(parent, seq) MPTREE_NODE_CHILDREN_IMPL(MPTREE_NODE_INVOKE_PARENT_1, parent, seq)

}
