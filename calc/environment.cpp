/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include "pch.hpp"

#ifndef NDEBUG
#include <fstream>
#endif

#include "error.hpp"
#include "utils.hpp"

#include "environment.hpp"

namespace calc {

class user_function_lookup {
public:
    explicit user_function_lookup(const std::vector<std::wstring> & names, const function_lookup & lookup)
        : names_(names), lookup_(lookup) {}

    func operator()(const std::wstring & name, size_t arity, bool lookupArguments, bool lowered) const
    {
        if(!lowered)
            return (*this)(mstd::to_lower_copy(name), arity, lookupArguments, true);
        else {
            std::vector<std::wstring>::const_iterator begin = names_.begin(), end = names_.end();
            std::vector<std::wstring>::const_iterator i = arity == 0 && lookupArguments ? std::find(begin, end, name) : end;
            if(i == end)
            {
                return lookup_(name, arity, false, true);
            } else {
                return func(stack_arg(i - begin), 0);
            }
        }
    }
private:
    std::vector<std::wstring> names_;
    function_lookup lookup_;
};

class deleter {
public:
    deleter(variable * vars, size_t & i)
        : vars_(vars), i_(i) {}

    ~deleter()
    {
        for(size_t j = 0; j != i_; ++j)
            vars_[j].~variable();
    }
private:
    variable * vars_;
    size_t & i_;
};

class user_function_invoker : public pre_program {
public:
    explicit user_function_invoker(const program & impl, std::vector<pre_program*> & args)
        : impl_(impl)
    {
        args_.swap(args);
    }

    variable run(void * context, variable * stack) const
    {
        size_t i = 0, size = args_.size();
        variable * vars = static_cast<variable*>(alloca(sizeof(variable) * (size + 1))) + 1;
        deleter d(vars, i);
        for(; i != size; ++i)
            new (vars + i) variable(args_[i]->run(context, stack));
        *mstd::pointer_cast<size_t*>(vars - 1) = size;
        return impl_(context, vars);
    }

    ~user_function_invoker()
    {
        for(std::vector<pre_program*>::iterator i = args_.begin(), end = args_.end(); i != end; ++i)
            delete *i;
    }
private:
    program impl_;
    std::vector<pre_program*> args_;
};

class user_function_compiler {
public:
    explicit user_function_compiler(const std::vector<std::wstring> & args, const compiler & f)
        : args_(args), f_(f) {}

    pre_program * operator()(std::vector<pre_program*> & args, const compiler_context & context) const
    {
        function_lookup lookup = user_function_lookup(args_, context.lookup);
        compiler_context newContext = { lookup, context.err, context.plugin };
        program p = f_(newContext);
        return !context.err ? new user_function_invoker(boost::move(p), args) : 0;
    }
private:
    std::vector<std::wstring> args_;
    compiler f_;
};

namespace {

#ifndef NDEBUG
std::ostream & functionsList()
{
    static std::ofstream result("functions.txt");
    return result;
}
#endif

}

void environment::do_add(const std::wstring & name, const func & f)
{
#ifndef NDEBUG
    functionsList() << mstd::utf8(name) << std::endl;
#endif
    map_[std::make_pair(boost::to_lower_copy(name), f.arity)] = f;
}

void environment::add(const std::wstring & name, const std::vector<std::wstring> & args, const compiler & f)
{
    do_add(name, func(user_function_compiler(args, f), args.size()));
}

func environment::find(const std::wstring & name, int arity, bool lowered) const
{
    BOOST_ASSERT(!lowered || name == mstd::to_lower_copy(name));
    return do_find(std::make_pair(lowered ? name : mstd::to_lower_copy(name), arity));
}

func environment::do_find(const Map::key_type & key) const
{
    Map::const_iterator i = map_.find(key);
    if(i == map_.end())
    {
        if(!lookup_.empty())
        {
            func result = lookup_(key.first, key.second, true, true);
            if(!result.function.empty())
                return result;
        }
        if(parent_)
            return parent_->do_find(key);
        else
            return func();
    } else
        return i->second;
}

}
