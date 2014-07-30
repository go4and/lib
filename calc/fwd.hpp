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
#include <memory>
#include <vector>

#include <boost/cstdint.hpp>

#include <boost/variant/variant.hpp>

#include <mstd/exception.hpp>
#endif

namespace calc {

class environment;
class pre_program;
struct func;
struct compiler_context;

class error;
typedef boost::int64_t number;
typedef boost::variant<number, std::wstring> variable;
typedef std::function<variable(void*, variable*)> program;
typedef std::function<func(const std::wstring& name, size_t arity, bool lookupArguments, bool lowered)> function_lookup;
typedef std::function<pre_program*(const std::wstring&, const compiler_context & context)> plugin_compiler;

struct compiler_context {
    const function_lookup & lookup;
    error & err;
    const plugin_compiler & plugin;
};

typedef std::function<program(const compiler_context & context)> compiler;

class pre_program : public boost::noncopyable {
public:
    virtual variable run(void * context, variable * stack) const = 0;

    virtual ~pre_program()
    {
    }
};

typedef std::auto_ptr<pre_program> pre_program_ptr;

struct func {
    typedef std::function<pre_program*(std::vector<pre_program*>&, const compiler_context & context)> function_type;

    function_type function;
    int arity;

    func() {}

    func(const function_type & f, int a)
        : function(f), arity(a) {}
};

}
