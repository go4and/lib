/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#define CALC_ERROR_PUBLIC_FUNCTIONS(r, data, p) CALC_ERROR_PUBLIC_FUNCTIONS_IMPL p
    BOOST_PP_SEQ_FOR_EACH(CALC_ERROR_PUBLIC_FUNCTIONS, ~, CALC_ERROR_PARAMETERS);
private:
    error_code code_;
    #define CALC_ERROR_MEMBERS_IMPL(type, name) boost::optional<type> BOOST_PP_CAT(name, _);
    #define CALC_ERROR_MEMBERS(r, data, p) CALC_ERROR_MEMBERS_IMPL p
    BOOST_PP_SEQ_FOR_EACH(CALC_ERROR_MEMBERS, ~, CALC_ERROR_PARAMETERS);
};

inline bool operator!(const error & err)
{
    return err.code() == error_none;
}

inline std::ostream & operator<<(std::ostream & out, const error & err)
{
    err.out(out);
    return out;
}

class exception : public boost::exception {
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
