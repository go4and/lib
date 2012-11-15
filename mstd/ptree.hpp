#pragma once

#include <boost/algorithm/string/predicate.hpp>

#include <boost/iterator/filter_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>

#include <boost/property_tree/ptree.hpp>

#include "singleton.hpp"

namespace mstd {

    namespace detail {

        template<class Key>
        class check_first {
        public:
            check_first() {}

            explicit check_first(const Key & key)
                : key_(key) {}

            template<class T>
            bool operator()(const T & t) const
            {
                return t.first == key_;
            }
        private:
            Key key_;
        };

        template<class Res>
        class extract_second {
        public:
            typedef Res & result_type;

            extract_second() {}

            template<class P>
            result_type operator()(P & src) const
            {
                return src.second;
            }
        };

    }

template<class Tree>
inline Tree & checked_get(Tree & inp, const typename Tree::path_type & path)
{
    boost::optional<Tree &> child = inp.get_child_optional(path);
    if(child)
        return child.get();
    return inp.put_child(path, Tree());
}

template<class Tree>
inline const Tree & checked_get(const Tree & inp, const typename Tree::path_type & path)
{
    boost::optional<const Tree&> child = inp.get_child_optional(path);
    if(child)
        return child.get();
    return mstd::default_instance<Tree>();
}

template<class Tree>
void merge(Tree & out, const Tree & inp)
{
    for(const typename Tree::const_iterator i = inp.begin(), end = inp.end(); i != end; ++i)
        out.put_child(i->first, i->second);
}

template<class Tree>
struct tree_traits {
    typedef typename boost::mpl::if_<boost::is_const<Tree>, typename Tree::const_iterator, typename Tree::iterator>::type iterator;
    typedef typename Tree::key_type key_type;
    typedef boost::filter_iterator<detail::check_first<key_type>, iterator> filter_iterator;
    typedef boost::transform_iterator<detail::extract_second<Tree>, filter_iterator> transform_iterator;
    typedef boost::iterator_range<transform_iterator> iterator_range;

    static transform_iterator make_iterator(const key_type & key, iterator i, iterator end)
    {
        return transform_iterator(filter_iterator(detail::check_first<key_type>(key), i, end), detail::extract_second<Tree>());
    }

    static iterator_range make_range(const key_type & key, iterator i, iterator end)
    {
        return iterator_range(make_iterator(key, i, end), make_iterator(key, end, end));
    }
};

template<class Tree>
typename tree_traits<Tree>::iterator_range subs(Tree & inp, const typename Tree::path_type & path, const typename Tree::key_type & key)
{
    typedef tree_traits<Tree> traits;
    
    typedef typename boost::mpl::if_<boost::is_const<Tree>, const Tree&, Tree&>::type Ref;
    boost::optional<Ref> child = inp.get_child_optional(path);
    if(child)
        return traits::make_range(key, child.get().begin(), child.get().end());
    else
        return traits::make_range(typename Tree::key_type(), inp.end(), inp.end());
}

template<class Ch>
struct bool_translator_base {
    typedef std::basic_string<Ch> internal_type;
    typedef bool external_type;

    boost::optional<bool> get_value(const internal_type & v)
    {
        if(boost::equals(v, "true") || boost::equals(v, "1"))
            return true;
        else if(boost::equals(v, "false") || boost::equals(v, "0"))
            return false;
        return boost::optional<bool>();
    }

    boost::optional<internal_type> put_value(bool inp)
    {
        return inp ? from_cstr("true") : from_cstr("false");
    }

    internal_type from_cstr(const char * str)
    {
        return internal_type(str, str + strlen(str));
    }
};

typedef bool_translator_base<char> bool_translator;
typedef bool_translator_base<wchar_t> wbool_translator;

void convert(const boost::property_tree::ptree & in, boost::property_tree::wptree & out);
void convert(const boost::property_tree::wptree & in, boost::property_tree::ptree & out);
void dumpTree(std::ostream & out, const boost::property_tree::ptree & tree, int ident = 0);
void dumpTree(std::ostream & out, const boost::property_tree::wptree & tree, int ident = 0);

template<class PTree>
class PTreeDump {
public:
    explicit PTreeDump(const PTree & tree)
        : tree_(&tree)
    {
    }

    void dump(std::ostream & out) const
    {
        dumpTree(out, *tree_);
    }
private:
    const PTree * tree_;
};

template<class PTree>
std::ostream & operator<<(std::ostream & out, const PTreeDump<PTree> & dump)
{
    dump.dump(out);
    return out;
}

inline PTreeDump<boost::property_tree::ptree> dump(const boost::property_tree::ptree & tree) { return PTreeDump<boost::property_tree::ptree>(tree); }
inline PTreeDump<boost::property_tree::wptree> dump(const boost::property_tree::wptree & tree) { return PTreeDump<boost::property_tree::wptree>(tree); }

}
