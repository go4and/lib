#pragma once

namespace calc {

class exception : public std::exception {
public:
    const char * what() const throw()
    {
        return "generic calculator failure";
    }
};

class parse_exception : public exception {
};

class invalid_function : public parse_exception {
public:
    explicit invalid_function(const std::string & name)
        : name_(name) {}

    ~invalid_function() throw() {}

    const std::string & name() const
    {
        return name_;
    }
private:
    std::string name_;
};

class undefined_function : public invalid_function {
public:
    explicit undefined_function(const std::string & name)
        : invalid_function(name) {}

    const char * what() const throw()
    {
        return "undefined function";
    }
};

class invalid_arity : public invalid_function {
public:
    invalid_arity(const std::string & name, int expected, int found)
        : invalid_function(name), expected_(expected), found_(found) {}
        
    int expected() const
    {
        return expected_;
    }
    
    int found() const
    {
        return found_;
    }

    const char * what() const throw()
    {
        return "invalid function arity";
    }
private:
    int expected_;
    int found_;
};

class calculation_exception : public exception {
};

class division_by_zero_exception : public calculation_exception {
public:
    const char * what() const throw()
    {
        return "division by zero";
    }
};

class comparison_exception : public calculation_exception {
public:
    comparison_exception(const char * op, const std::type_info & lhst, const std::type_info & rhst)
        : op_(op), lhst_(lhst), rhst_(rhst)
    {
    }
    
    const char * op() const
    {
        return op_;
    }
    
    const std::type_info & lhs_type() const
    {
        return lhst_;
    }
    
    const std::type_info & rhs_type() const
    {
        return rhst_;
    }

    const char * what() const throw()
    {
        return "comparison failed";
    }
private:
    const char * op_;
    const std::type_info & lhst_;
    const std::type_info & rhst_;
};

}
