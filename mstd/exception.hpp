#pragma once

#include <string>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4512)
#endif

#include <boost/version.hpp>

#include <boost/exception/exception.hpp>
#include <boost/exception/info.hpp>
#include <boost/exception/get_error_info.hpp>

#if !defined(MSTD_NO_ERROR_CODE)
#include <boost/system/error_code.hpp>
#endif

#include <boost/type_traits/is_base_of.hpp>

#include <boost/utility/enable_if.hpp>

#include "config.hpp"

#include "strings.hpp"

namespace mstd {

class tag_message;
typedef boost::error_info<tag_message, std::string> error_message;
class tag_errno;
typedef boost::error_info<tag_errno, int> error_no;
#if !defined(MSTD_NO_ERROR_CODE)
class tag_errcode;
typedef boost::error_info<tag_errno, boost::system::error_code> error_code;
#endif

inline error_message make_error_message(const std::string & text)
{
    return error_message(text);
}

inline error_message make_error_message(const std::wstring & text)
{
    return error_message(mstd::utf8(text));
}

class MSTD_DECL exception : public std::exception, public boost::exception {
public:
    exception() {}
    explicit exception(const boost::exception & exc)
        : boost::exception(exc) {}
    ~exception() throw () {}
};

template<class Tag, class Parent = exception>
class own_exception : public Parent {
public:
    own_exception() {}
    explicit own_exception(const boost::exception & exc)
        : exception(exc) {}
    ~own_exception() throw () {}
};

template<class OutException, class InException>
void rethrow(InException & input)
{
    throw OutException(input);
}

template<class E>
typename boost::enable_if<boost::is_base_of<std::exception, E>, const char*>::type
get_what(E & exc)
{
    return exc.what();
}

template<class E>
typename boost::disable_if<boost::is_base_of<std::exception, E>, const char*>::type
get_what(E & exc)
{
    std::exception * stdexc = dynamic_cast<std::exception*>(&exc);
    return stdexc ? stdexc->what() : "";
}

template<class E>
std::string get_error_message(E & exc)
{
#if BOOST_VERSION < 104100
    boost::shared_ptr<const std::string>
#else
    const std::string *
#endif
    msg = boost::get_error_info<error_message>(exc);
    if(msg)
        return *msg;
    else
        return get_what(exc);
}

template<class E>
std::wstring wget_error_message(E & exc)
{
    return mstd::deutf8(get_error_message(exc));
}

class MSTD_DECL BoostExceptionOut {
public:
    explicit BoostExceptionOut(boost::exception & exc)
        : exc_(&exc) {}

    void out(std::ostream & stream) const;
private:
    boost::exception * exc_;
};

inline BoostExceptionOut out_exception(boost::exception & exc)
{
    return BoostExceptionOut(exc);
}

inline std::ostream & operator<<(std::ostream & stream, const BoostExceptionOut & out)
{
    out.out(stream);
    return stream;
}

class MSTD_DECL StdExceptionOut {
public:
    explicit StdExceptionOut(std::exception & exc)
        : exc_(&exc) {}

    void out(std::ostream & stream) const;
private:
    std::exception * exc_;
};

inline StdExceptionOut out_exception(std::exception & exc)
{
    return StdExceptionOut(exc);
}

template<class E>
inline typename boost::enable_if<boost::is_base_of<boost::exception, E>, BoostExceptionOut>::type
out_exception(E & e)
{
    return BoostExceptionOut(e);
}

inline std::ostream & operator<<(std::ostream & stream, const StdExceptionOut & out)
{
    out.out(stream);
    return stream;
}

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

}
