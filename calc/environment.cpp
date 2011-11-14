#include "pch.hpp"

#ifndef NDEBUG
#include <fstream>
#endif

#include "utils.hpp"

#include "environment.hpp"

namespace ph = boost::phoenix;
using namespace boost::phoenix::arg_names;

namespace calc {

class user_function_lookup {
public:
    explicit user_function_lookup(const std::vector<std::wstring> & names, const function_lookup & lookup)
        : names_(names), lookup_(lookup) {}

    func operator()(const std::wstring & name, size_t arity) const
    {
        std::vector<std::wstring>::const_iterator begin = names_.begin(), end = names_.end();
        std::vector<std::wstring>::const_iterator i = arity == 0 ? std::find(begin, end, name) : end;
        if(i == end)
        {
            return lookup_(name, arity);
        } else {
            return func(stack_arg(i - begin), 0);
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

class user_function_invoker {
public:
    explicit user_function_invoker(const program & impl, const std::vector<program> & args)
        : impl_(impl), args_(args) {}

    variable operator()(void * context, variable * stack) const
    {
        size_t i = 0, size = args_.size();
        variable * vars = static_cast<variable*>(alloca(sizeof(variable) * size));
        deleter d(vars, i);
        for(; i != size; ++i)
            new (vars + i) variable(args_[i](context, stack));
        return impl_(context, vars);
    }
private:
    program impl_;
    std::vector<program> args_;
};

class user_function_compiler {
public:
    explicit user_function_compiler(const std::vector<std::wstring> & args, const compiler & f)
        : args_(args), f_(f) {}

    program operator()(const std::vector<program> & args, const function_lookup & lookup) const
    {
        return user_function_invoker(f_(user_function_lookup(args_, lookup)), args);
    }
private:
    std::vector<std::wstring> args_;
    compiler f_;
};

#ifndef NDEBUG
std::ostream & functionsList()
{
    static std::ofstream result("functions.txt");
    return result;
}
#endif

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

func environment::find(const std::wstring & name, int arity) const
{
    return do_find(std::make_pair(boost::to_lower_copy(name), arity));
}

func environment::do_find(const Map::key_type & key) const
{
    Map::const_iterator i = map_.find(key);
    if(i == map_.end())
    {
        if(parent_)
            return parent_->do_find(key);
        else
            return func();
    } else
        return i->second;
}

}
