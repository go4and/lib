/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#include <boost/property_tree/ptree.hpp>

namespace mnet {

class ParseError {
    struct Dummy { void f() {} };
public:
    typedef void (Dummy::*safe_bool)();

    ParseError()
    {
    }

    void reset()
    {
        error_.clear();
    }

    void init(const char * message)
    {
        error_ = message;
    }

    void init(const std::string & message)
    {
        error_ = message;
    }

    const std::string & message() const
    {
        return error_;
    }

    operator safe_bool() const
    {
        return !error_.empty() ? &Dummy::f : 0;
    }   
    
    bool operator!() const
    {
        return error_.empty();
    } 
private:
    std::string error_;
};

void parseJSON(const char * data, size_t len, boost::property_tree::ptree & tree, ParseError & error);
inline void parseJSON(const std::string & str, boost::property_tree::ptree & tree, ParseError & error) { parseJSON(str.c_str(), str.length(), tree, error); }
void parseJSON(const boost::filesystem::path & path, boost::property_tree::ptree & tree, ParseError & error);

}
