#pragma once

#if !defined(BUILDING_CALC)
#include <vector>

#include <boost/cstdint.hpp>
#include <boost/function.hpp>

#include <boost/variant/variant.hpp>

#include <mstd/exception.hpp>
#endif

namespace calc {

class environment;
struct func;

typedef boost::int64_t number;
typedef boost::variant<number, std::wstring> variable;
typedef boost::function<variable(void*, variable*)> program;
typedef boost::function<func(const std::wstring&, size_t)> function_lookup;
typedef boost::function<program(const function_lookup & lookup)> compiler;

struct func {
    typedef boost::function<program(const std::vector<program>&, const function_lookup&)> function_type;

    function_type function;
    int arity;

    func() {}

    func(const function_type & f, int a)
        : function(f), arity(a) {}
};

}
