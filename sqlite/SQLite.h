#pragma once

#include <string>

#include <boost/noncopyable.hpp>
#include <boost/type_traits/is_same.hpp>

#include <mstd/cstdint.hpp>

#if !SQLITE_NO_EXCEPTIONS
#error What a fuck?
#include <mstd/exception.hpp>
#endif

struct sqlite3;
struct sqlite3_stmt;

namespace sqlite {

class SQLite;

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

class SQLiteStatement : private boost::noncopyable {
public:
#if !SQLITE_NO_EXCEPTIONS
    SQLiteStatement(SQLite & db, const std::string & sql);
    SQLiteStatement(SQLite & db, const std::wstring & sql);
#endif
    SQLiteStatement(SQLite & db, const std::string & sql, ErrorCode & ec);
    SQLiteStatement(SQLite & db, const std::wstring & sql, ErrorCode & ec);

    ~SQLiteStatement();

#if !SQLITE_NO_EXCEPTIONS
    void reset();
    bool step();
#endif
    void reset(ErrorCode & ec);
    bool step(ErrorCode & ec);

    void bindInt64(int index, int64_t value);
    void bindInt(int index, int32_t value);
    void bindString(int index, const std::string & value);
    
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
    bool isNull(int col);
private:
    SQLite & db_;
    sqlite3_stmt * handle_;
#if !defined(MLOG_NO_LOGGING)
    std::string sql_;
#endif
};

class SQLite : private boost::noncopyable {
public:
#if !SQLITE_NO_EXCEPTIONS
    SQLite(const std::string & filename);
    SQLite(const std::wstring & filename);
#endif

    SQLite(const std::string & filename, ErrorCode & ec);
    SQLite(const std::wstring & filename, ErrorCode & ec);
    
    SQLite();
    void open(const std::string & filename, ErrorCode & ec);
    void open(const std::wstring & filename, ErrorCode & ec);

    int64_t lastInsertRowId();

    sqlite3 * handle();
#if !SQLITE_NO_EXCEPTIONS
    void exec(const std::string & sql);
#endif
    void exec(const std::string & sql, ErrorCode & ec);

    ~SQLite();
private:
    sqlite3 * handle_;
};

class Transaction : public boost::noncopyable {
public:
#if !SQLITE_NO_EXCEPTIONS
    explicit Transaction(SQLite & db);
#endif
    explicit Transaction(SQLite & db, ErrorCode & ec);

    ~Transaction();

#if !SQLITE_NO_EXCEPTIONS
    void commit();
#endif
    void commit(ErrorCode & ec);
private:
    SQLite * db_;
    bool ok_;
};

#if !SQLITE_NO_EXCEPTIONS
typedef mstd::own_exception<SQLite> SQLiteException;
#endif

}
