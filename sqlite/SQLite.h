/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

#include <string>

#include <boost/noncopyable.hpp>
#include <boost/type_traits/is_same.hpp>

#include <mstd/cstdint.hpp>

#if !SQLITE_NO_EXCEPTIONS
#include <mstd/exception.hpp>
#endif

struct sqlite3;
struct sqlite3_stmt;

namespace sqlite {

class DB;

class ErrorCode {
public:
    ErrorCode() : err_(0)
    {}

    explicit ErrorCode(int err, const std::string & message)
        : err_(err), message_(message) {}
    
    void reset(int err, const std::string & message)
    {
        err_ = err;
        message_ = message;
    }
    
    typedef int ErrorCode::*unspecified_bool_type;

    operator unspecified_bool_type() const
    {
        return err_ ? &ErrorCode::err_ : 0;
    }

    int err() const { return err_; }
    const std::string & message() const { return message_; }

#if !SQLITE_NO_EXCEPTIONS    
    void check_();
#endif
private:
    int err_;
    std::string message_;
};

inline std::ostream & operator<<(std::ostream & out, const ErrorCode & ec)
{
    if(ec)
        return out << "{err=" << ec.err() << ",message=" << ec.message() << '}';
    else
        return out << "{no error}";
}

struct Blob {
    const char * data;
    int len;
};

inline Blob blob(const char * data, int len)
{
    Blob result = { data, len };
    return result;
}

class Statement : private boost::noncopyable {
public:
#if !SQLITE_NO_EXCEPTIONS
    Statement(DB & db, const std::string & sql);
    Statement(DB & db, const std::wstring & sql);
#endif
    Statement(DB & db, const std::string & sql, ErrorCode & ec);
    Statement(DB & db, const std::wstring & sql, ErrorCode & ec);

    ~Statement();

#if !SQLITE_NO_EXCEPTIONS
    void reset();
    bool step();
#endif
    void reset(ErrorCode & ec);
    bool step(ErrorCode & ec);

    void bindInt64(int index, int64_t value);
    void bindInt(int index, int32_t value);
    void bindString(int index, const std::string & value);
    void bindString(int index, const char * value);
    void bindBlob(int index, const char * data, int len);
    inline void bindBlob(int index, const Blob & blob) { bindBlob(index, blob.data, blob.len); }
    
    template<class T, class S>
    typename boost::enable_if<boost::is_same<T, int64_t>, void>::type
    bind(int index, S value)
    {
        bindInt64(index, value);
    }

    template<class T, class S>
    typename boost::enable_if<boost::is_same<T, int32_t>, void>::type
    bind(int index, S value)
    {
        bindInt(index, value);
    }

    int32_t getInt(int col);
    int64_t getInt64(int col);
    double getDouble(int col);
    std::string getString(int col);
    std::wstring getWString(int col);
    Blob getBlob(int col);
    bool isNull(int col);
private:
    DB & db_;
    sqlite3_stmt * handle_;
#if !defined(MLOG_NO_LOGGING)
    std::string sql_;
#endif
};

class DB : private boost::noncopyable {
public:
#if !SQLITE_NO_EXCEPTIONS
    DB(const std::string & filename);
    DB(const std::wstring & filename);
#endif

    DB(const std::string & filename, ErrorCode & ec);
    DB(const std::wstring & filename, ErrorCode & ec);
    
    DB();
    void open(const std::string & filename, ErrorCode & ec);
    void open(const std::wstring & filename, ErrorCode & ec);

    int64_t lastInsertRowId();

    sqlite3 * handle();
#if !SQLITE_NO_EXCEPTIONS
    void exec(const std::string & sql);
#endif
    void exec(const std::string & sql, ErrorCode & ec);

    ~DB();
private:
    sqlite3 * handle_;
};

class Transaction : public boost::noncopyable {
public:
#if !SQLITE_NO_EXCEPTIONS
    explicit Transaction(DB & db);
#endif
    explicit Transaction(DB & db, ErrorCode & ec);

    ~Transaction();

#if !SQLITE_NO_EXCEPTIONS
    void commit();
#endif
    void commit(ErrorCode & ec);
private:
    DB * db_;
    bool ok_;
};

#if !SQLITE_NO_EXCEPTIONS
typedef mstd::own_exception<DB> Exception;
#endif

}
