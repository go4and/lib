#include "pch.h"

#include "Error.h"

namespace mcrypt {

namespace {

std::string getErrorMessage(unsigned long error)
{
    if(error == Error::unknown)
        return "Unknown error";
    char buffer[0x1000];
    ERR_load_crypto_strings();
    
    ERR_error_string_n(error, buffer, sizeof(buffer));
    return buffer;
}

}

Error::Error(unsigned long code)
    : code_(code) {}
    
const std::string & Error::message() const
{
    if(what_.empty() && code_)
        what_ = getErrorMessage(code_);
    return what_;
}

bool Error::initError()
{
    code_ = ERR_get_error();
    if(code_ == 0)
        code_ = unknown;
    what_.clear();
    return true;
}

bool Error::checkError(int result)
{
    if(result == -1)
        return initError();
    return false;
}

bool Error::checkResult(int result)
{
    if(result != 1)
        return initError();
    return false;
}

}
