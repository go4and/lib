#include <string.h>

#include <mstd/pointer_cast.hpp>
#if !defined(MLOG_NO_LOGGING)
#include <mstd/performance_timer.hpp>
#endif
#include <mstd/strings.hpp>
#include <mstd/utf8.hpp>

#include <mlog/Logging.h>

#if defined(_MSC_VER)
#include "internal/sqlite3.h"
#else
#include <sqlite3.h>
#endif

#include "SQLite.h"

MLOG_DECLARE_LOGGER(sqlite);

namespace sqlite {

void resetEC(ErrorCode & ec, int err, sqlite3 * handle)
{
    ec.reset(err, (err == SQLITE_OK || !handle) ? std::string() : sqlite3_errmsg(handle));
}

//////////////////////////////////////////////////////////////////////////
// class SafeHolder
//////////////////////////////////////////////////////////////////////////

template<class Object, int (*Finalizer)(Object * object)>
class SafeHolder {
public:
    SafeHolder()
        : object_(NULL)
    {
    }
    
    ~SafeHolder()
    {
        if(object_)
            Finalizer(object_);
    }
    
    Object * operator*()
    {
        return handle();
    }
    
    Object * handle()
    {
        return object_;
    }
    
    Object ** writeHandle()
    {
        return & object_;
    }
private:
    Object * object_;
};

#if !SQLITE_NO_EXCEPTIONS
void ErrorCode::check_()
{
    if(err_ != SQLITE_OK)
        BOOST_THROW_EXCEPTION(Exception() << mstd::error_message(message_) << mstd::error_no(err_));
}
#endif

sqlite3 * sqliteOpen(const std::string & filename, ErrorCode & ec)
{
    sqlite3 * result = 0;
    int err = sqlite3_open(filename.c_str(), &result);
    resetEC(ec, err, result);
    if(ec)
    {
        sqlite3_close(result);
        result = 0;
    }
    return result;
}

//////////////////////////////////////////////////////////////////////////
// class DB
//////////////////////////////////////////////////////////////////////////

#if !SQLITE_NO_EXCEPTIONS
DB::DB(const std::string & filename)
{
    ErrorCode ec;
    handle_ = sqliteOpen(filename, ec);
    ec.check_();
}

DB::DB(const std::wstring & filename)
{
    ErrorCode ec;
    handle_ = sqliteOpen(mstd::utf8(filename), ec);
    ec.check_();
}
    
void DB::exec(const std::string & sql)
{   
    Statement stmt(*this, sql);
    stmt.step();
}
#endif

DB::DB() : handle_(0)
{
}

DB::DB(const std::string & filename, ErrorCode & ec)
    : handle_(sqliteOpen(filename, ec)) 
{
}

DB::DB(const std::wstring & filename, ErrorCode & ec)
    : handle_(sqliteOpen(mstd::utf8(filename), ec)) {}

void DB::open(const std::string & filename, ErrorCode & ec)
{
    if(handle_)
        sqlite3_close(handle_);
    handle_ = sqliteOpen(filename, ec);
}

void DB::open(const std::wstring & filename, ErrorCode & ec)
{
    if(handle_)
        sqlite3_close(handle_);
    handle_ = sqliteOpen(mstd::utf8(filename), ec);
}
    
void DB::exec(const std::string & sql, ErrorCode & ec)
{   
    Statement stmt(*this, sql, ec);
    if(!ec)
        stmt.step(ec);
}

sqlite3 * DB::handle()
{
    return handle_;
}

int64_t DB::lastInsertRowId()
{
    return sqlite3_last_insert_rowid(handle_);
}

DB::~DB()
{
    if(handle_)
        sqlite3_close(handle_);
}

//////////////////////////////////////////////////////////////////////////
// class Statement
//////////////////////////////////////////////////////////////////////////

sqlite3_stmt * sqlitePrepare(DB & db, const std::string & sql, ErrorCode & ec)
{
    const char * dummy;
    sqlite3_stmt * result = 0;
    resetEC(ec, sqlite3_prepare_v2(db.handle(), sql.c_str(), sql.length(), &result, &dummy), db.handle());
    if(ec)
    {
        sqlite3_finalize(result);
        result = 0;
    }
    return result;
}

#if !SQLITE_NO_EXCEPTIONS
Statement::Statement(DB & db, const std::string & sql)
    : db_(db)
#if !defined(MLOG_NO_LOGGING)
          , sql_(sql)
#endif
{
    ErrorCode ec;
    handle_ = sqlitePrepare(db, sql, ec);
    ec.check_();
}

Statement::Statement(DB & db, const std::wstring & sql)
    : db_(db)
#if !defined(MLOG_NO_LOGGING)
          , sql_(mstd::utf8(sql))
#endif
{
    ErrorCode ec;
    handle_ = sqlitePrepare(db, sql_, ec);
    ec.check_();
}
#endif

Statement::Statement(DB & db, const std::string & sql, ErrorCode & ec)
    : db_(db), handle_(sqlitePrepare(db, sql, ec))
#if !defined(MLOG_NO_LOGGING)
          , sql_(sql)
#endif
{
}

Statement::Statement(DB & db, const std::wstring & sql, ErrorCode & ec)
    : db_(db), handle_(sqlitePrepare(db, mstd::utf8(sql), ec))
#if !defined(MLOG_NO_LOGGING)
          , sql_(mstd::utf8(sql))
#endif
{
}

Statement::~Statement()
{
    if(handle_)
        sqlite3_finalize(handle_);
}

void Statement::bindInt(int index, int32_t value)
{
    sqlite3_bind_int(handle_, index, value);
}

void Statement::bindInt64(int index, int64_t value)
{
    sqlite3_bind_int64(handle_, index, value);
}

void Statement::bindString(int index, const char * value)
{
    sqlite3_bind_text(handle_, index, value, strlen(value), SQLITE_STATIC);
}

void Statement::bindString(int index, const std::string & value)
{
    sqlite3_bind_text(handle_, index, value.c_str(), value.length(), SQLITE_STATIC);
}

void Statement::bindBlob(int index, const char * data, int len)
{
    sqlite3_bind_blob(handle_, index, data, len, SQLITE_STATIC);
}

void Statement::reset(ErrorCode & ec)
{
    resetEC(ec, sqlite3_reset(handle_), db_.handle());
}

bool Statement::step(ErrorCode & ec)
{
#if !defined(__APPLE__) && !defined(MLOG_NO_LOGGING)
    mstd::performance_timer ptimer;
#endif
    int err = sqlite3_step(handle_);
#if !defined(__APPLE__) && !defined(MLOG_NO_LOGGING)
    uint64_t cur = ptimer.microseconds();
    if(cur > 100000)
        MLOG_MESSAGE(Warning, "Slow query: " << cur << ", sql: " << sql_);
#endif
    if(err == SQLITE_ROW)
    {
        ec.reset(0, std::string());
        return true;
    } else if(err == SQLITE_DONE)
    {
        ec.reset(0, std::string());
        return false;
    } else {
        resetEC(ec, err, db_.handle());
        return false;
    }
}

#if !SQLITE_NO_EXCEPTIONS
bool Statement::step()
{
    ErrorCode ec;
    bool result = step(ec);
    ec.check_();
    return result;
}
#endif

int32_t Statement::getInt(int col)
{
    return sqlite3_column_int(handle_, col);
}

int64_t Statement::getInt64(int col)
{
    return sqlite3_column_int64(handle_, col);
}

double Statement::getDouble(int col)
{
    return sqlite3_column_int(handle_, col);
}

bool Statement::isNull(int col)
{
    return sqlite3_column_type(handle_, col) == SQLITE_NULL;
}

std::string Statement::getString(int col)
{
    return mstd::pointer_cast<const char*>(sqlite3_column_text(handle_, col));
}

std::wstring Statement::getWString(int col)
{
    const char * res = mstd::pointer_cast<const char*>(sqlite3_column_text(handle_, col));
    size_t len = strlen(res);
    wchar_t * buf = mstd::pointer_cast<wchar_t*>(alloca(sizeof(wchar_t) * (len + 1)));
    wchar_t * end = mstd::deutf8(res, res + len, buf);
    return std::wstring(buf, end);
}

Blob Statement::getBlob(int col)
{
    Blob result;
    result.data = static_cast<const char*>(sqlite3_column_blob(handle_, col));
    result.len = sqlite3_column_bytes(handle_, col);
    return result;
}

Transaction::Transaction(DB & db, ErrorCode & ec)
    : db_(&db), ok_(false)
{
    db_->exec("begin", ec);
    if(ec)
        db_ = 0;
}

#if !SQLITE_NO_EXCEPTIONS
Transaction::Transaction(DB & db)
    : db_(&db), ok_(false)
{
    ErrorCode ec;
    db_->exec("begin", ec);
    if(ec)
        db_ = 0;
    ec.check_();
}
#endif

Transaction::~Transaction()
{
    if(db_ && !ok_)
    {
        ErrorCode ec;
        db_->exec("rollback" ,ec);
    }
}

void Transaction::commit(ErrorCode & ec)
{
    if(db_)
    {
        db_->exec("commit", ec);
        ok_ = !ec;
    } else
        ok_ = true;
}

#if !SQLITE_NO_EXCEPTIONS
void Transaction::commit()
{
    ErrorCode ec;
    commit(ec);
    ec.check_();
}
#endif

}
