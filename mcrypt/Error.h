/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

#include "Config.h"

namespace mcrypt {

class MCRYPT_DECL Error {
private:
    typedef void (*unspecified_bool_type)();
    static void unspecified_bool_true() {}
public:
    static const unsigned long unknown = static_cast<unsigned long>(-1);

    explicit Error(unsigned long code = 0);

    const std::string & message() const;
    unsigned long code() const { return code_; }

    void reset() { code_ = 0; what_.clear(); }

    operator unspecified_bool_type() const
    { 
        return !code_ ? 0 : unspecified_bool_true;
    }

    bool operator!() const
    {
        return !code_;
    }

    bool checkError(int res);
    bool checkResult(int res);
private:
    bool initError();

    unsigned long code_;
    mutable std::string what_;
};

}
