/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#define MPTREE_NODE_PARSE(parentInvoker, parent, seq) \
    mptree::parser_state child_parser(const char * name, size_t len, bool final) \
    { \
        using mptree::make_parser; \
        using mptree::make_unparsed_parser; \
        if(!name) { complete(); return mptree::parser_state(0, 0, 0); } \
        parentInvoker((mptree::parser_state temp = parent::child_parser(name, len, false); if(temp.data) return temp;)); \
        BOOST_PP_SEQ_FOR_EACH(MPTREE_NODE_PARSE_CHILD_ITEM, ~, seq); \
        if(!final) \
            return mptree::parser_state(0, 0, 0); \
        else \
            return make_unparsed_parser(name, len, unparsed_); \
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
