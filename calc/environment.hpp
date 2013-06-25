/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#if !defined(BUILDING_CALC)
#include <boost/algorithm/string/case_conv.hpp>

#include <boost/array.hpp>
#include <boost/unordered_map.hpp>
#endif

#include "config.hpp"
#include "fwd.hpp"
#include "environment_op.hpp"

namespace calc {

class CALC_DECL environment : public boost::noncopyable {
public:
    explicit environment(const environment * parent = 0, const function_lookup & lookup = function_lookup())
        : parent_(parent), lookup_(lookup) {}

    template<class F>
    void add(const std::wstring & name, const F & f)
    {
        check<typename detail::template traits<F>::context_argument>();
        do_add(name, func(detail::func_compiler<F>(f), detail::traits<F>::arity));
    }

    void add(const std::wstring & name, const std::vector<std::wstring> & args, const compiler & f);
    func find(const std::wstring & name, int arity) const;

    const environment * parent() const
    {
        return parent_;
    }

    void parent(const environment * value)
    {
        parent_ = value;
    }

    void clear()
    {
        map_.clear();
    }
private:
    typedef boost::unordered_map<std::pair<std::wstring, size_t>, func> Map;

    template<class T>
    void do_check()
    {
        BOOST_STATIC_ASSERT((boost::is_convertible<void*, T>::value));
    }

    template<class T>
    void check()
    {
        do_check<T>();
    }

    func do_find(const Map::key_type & key) const;
    void do_add(const std::wstring & name, const func & f);

    const environment * parent_;
    Map map_;
    function_lookup lookup_;
};

}
