#pragma once

#include <boost/lexical_cast.hpp>

namespace mstd {

class bad_lexical_cast : public boost::exception, public std::exception {
public:
    bad_lexical_cast(const boost::bad_lexical_cast & exc)
        : source(&exc.source_type()), target(&exc.target_type())
    {}

    const std::type_info &source_type() const
    {
        return *source;
    }
    const std::type_info &target_type() const
    {
        return *target;
    }

    virtual const char *what() const throw()
    {
        return "bad lexical cast: "
               "source type value could not be interpreted as target";
    }
    virtual ~bad_lexical_cast() throw()
    {
    }
private:
    const std::type_info *source;
    const std::type_info *target;
};

class tag_source;
typedef boost::error_info<tag_source, std::wstring> error_source_value;
class tag_name;
typedef boost::error_info<tag_source, std::string> error_source_name;

template<typename Target>
inline Target lexical_cast(const std::string & name, const std::wstring & arg)
{
    try {
        return boost::lexical_cast<Target>(arg);
    } catch(boost::bad_lexical_cast & src) {
        BOOST_THROW_EXCEPTION(bad_lexical_cast(src) << error_source_name(name) << error_source_value(arg));
        throw;
    }
}

}
