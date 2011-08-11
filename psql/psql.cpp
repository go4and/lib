#if defined(_MSC_VER)
#pragma warning(disable: 4996)

#include <WinSock2.h>
#endif

#include <boost/date_time/posix_time/posix_time_io.hpp>

#include <mstd/hton.hpp>
#include <mstd/null.hpp>
#include <mstd/pointer_cast.hpp>

#include <mlog/Logging.h>

#include "psql.h"

MLOG_DECLARE_LOGGER(psql);

namespace psql {

namespace {

void write2(char *& pos, boost::int16_t value)
{
    *mstd::pointer_cast<boost::int16_t*>(pos) = mstd::hton(value);
    pos += 2;
}

void write4(char *& pos, boost::int32_t value)
{
    *mstd::pointer_cast<boost::int32_t*>(pos) = mstd::hton(value);
    pos += 4;
}

void write8(char *& pos, boost::int64_t value)
{
    *mstd::pointer_cast<boost::int64_t*>(pos) = mstd::hton(value);
    pos += 8;
}

}

PGConnHolder::PGConnHolder(const std::string & cmd)
{
    conn_ = PQconnectStart(cmd.c_str());
}

PGConnHolder::~PGConnHolder()
{
    if(conn_)
        PQfinish(conn_);
}

PGconn * PGConnHolder::conn()
{
    return conn_;
}

////////////////////////////////////////////////////////////////////////////////
// class Connection
////////////////////////////////////////////////////////////////////////////////

Connection::Connection(const std::string & cmd)
    : PGConnHolder(cmd)
{
    if(!conn() || PQstatus(conn()) == CONNECTION_BAD)
        BOOST_THROW_EXCEPTION(ConnectionException());

    int socket = PQsocket(conn());
    PostgresPollingStatusType status = PGRES_POLLING_WRITING;
    while(status != PGRES_POLLING_OK)
    {
        fd_set sockets;
        FD_ZERO(&sockets);
        FD_SET(socket, &sockets);

        int result = 0;
        timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
        if(status == PGRES_POLLING_READING)
        {
            result = select(socket + 1, &sockets, 0, 0, &timeout);
        } else if(status == PGRES_POLLING_WRITING)
        {
            result = select(socket + 1, 0, &sockets, 0, &timeout);
        }
        
        if(result > 0)
        {
            status = PQconnectPoll(conn());

            MLOG_MESSAGE(Debug, "Poll result: " << status << ", conn: " << conn());

            if(status == PGRES_POLLING_FAILED)
            {
                ConnStatusType connStatus = PQstatus(conn());
                BOOST_THROW_EXCEPTION(ConnectionException() << mstd::error_message(PQerrorMessage(conn())) << ConnStatus(connStatus));
            }
        } else if(result < 0)
            MLOG_MESSAGE(Warning, "Select failed: " << result);
    }

    MLOG_MESSAGE(Notice, "DB Connected: " << conn());
}

Connection::~Connection()
{
}

const boost::posix_time::time_duration timeout = boost::posix_time::seconds(15);

void Connection::checkResult(const char * query, Result & result, bool canHaveErrors, const boost::posix_time::ptime & start)
{
    boost::posix_time::ptime stop = boost::posix_time::microsec_clock::universal_time();
    boost::posix_time::time_duration passed = stop - start;
    if(passed > timeout)
        MLOG_MESSAGE(Error, "slow query: " << query << ", passed: " << passed);
    if(!result.success())
    {
        if(!canHaveErrors)
            MLOG_MESSAGE(Error, "exec failed: " << result.status() << ", msg: " << result.error() << ", query: " << query);
        else
            MLOG_MESSAGE(Notice, "exec failed: " << result.status() << ", msg: " << result.error() << ", query: " << query);
        BOOST_THROW_EXCEPTION(ExecException() << mstd::error_message(result.error()));
    }
}

Result Connection::exec(const char * query, bool canHaveErrors)
{
    MLOG_MESSAGE(Debug, "exec(" << query << ")");

    boost::posix_time::ptime start = boost::posix_time::microsec_clock::universal_time();
    Result result(PQexecParams(conn(), query, 0, 0, 0, 0, 0, 1));
    checkResult(query, result, canHaveErrors, start);

    MLOG_MESSAGE(Debug, "exec succeeded");
    return move(result);
}

Result Connection::exec(const std::string & query, bool canHaveErrors)
{
    return exec(query.c_str(), canHaveErrors);
}

void Connection::execVoid(const char * query, bool canHaveErrors)
{
    MLOG_MESSAGE(Debug, "execVoid(" << query << ")");

    boost::posix_time::ptime start = boost::posix_time::microsec_clock::universal_time();
    Result result(PQexecParams(conn(), query, 0, 0, 0, 0, 0, 0));
    checkResult(query, result, canHaveErrors, start);

    MLOG_MESSAGE(Debug, "exec succeeded");
}

void Connection::execVoid(const std::string & query, bool canHaveErrors)
{
    execVoid(query.c_str(), canHaveErrors);
}

Result Connection::exec(const char * query, size_t size, const char * const * values, int * lengths, bool canHaveErrors)
{
    MLOG_MESSAGE(Debug, "exec(" << query << ", " << size << ")");
    
    int * formats = static_cast<int*>(alloca(size * sizeof(int)));
    std::fill_n(formats, size, 1);

    boost::posix_time::ptime start = boost::posix_time::microsec_clock::universal_time();
    Result result(PQexecParams(conn(), query, size, 0,
                               values, lengths, formats, 1));

    checkResult(query, result, canHaveErrors, start);

    MLOG_MESSAGE(Debug, "exec succeeded");
    return move(result);
}

void Connection::execVoid(const char * query, size_t size, const char * const * values, int * lengths, bool canHaveErrors)
{
    MLOG_MESSAGE(Debug, "execVoid(" << query << ", " << size << ")");
    
    int * formats = static_cast<int*>(alloca(size * sizeof(int)));
    std::fill_n(formats, size, 1);

    boost::posix_time::ptime start = boost::posix_time::microsec_clock::universal_time();
    Result result(PQexecParams(conn(), query, size, 0,
                               values, lengths, formats, 0));

    checkResult(query, result, canHaveErrors, start);

    MLOG_MESSAGE(Debug, "exec succeeded");
}

////////////////////////////////////////////////////////////////////////////////
// class Transaction
////////////////////////////////////////////////////////////////////////////////

Transaction::Transaction(psql::Connection & conn, IsolationLevel level)
    : conn_(conn), commited_(boost::indeterminate)
{
    switch(level) {
    case ilSerializable:
        conn_.exec("begin isolation level serializable", false);
        break;
    case ilReadCommitted:
        conn_.exec("begin isolation level read committed", false);
        break;
    default:
        conn_.exec("begin", false);
        break;
    }
}

void Transaction::commit()
{
    conn_.exec("commit", false);
    commited_ = true;
}

void Transaction::rollback()
{
    conn_.exec("rollback", false);
    commited_ = false;
}

Transaction::~Transaction()
{
    if(indeterminate(commited_))
        conn_.exec("rollback", false);
}

////////////////////////////////////////////////////////////////////////////////
// class Result
////////////////////////////////////////////////////////////////////////////////

ParametricExecution::ParametricExecution(Connection & conn)
    : conn_(conn) {}

ParametricExecution::~ParametricExecution()
{
    for(std::vector<char*>::const_iterator i = buffers_.begin(), end = buffers_.end(); i != end; ++i)
        delete [] *i;
}

void ParametricExecution::clear()
{
    longInts_.clear();
    ints_.clear();
    smallInts_.clear();
    values_.clear();
    lengths_.clear();
    for(std::vector<char*>::const_iterator i = buffers_.begin(), end = buffers_.end(); i != end; ++i)
        delete [] *i;
    buffers_.clear();
}

Result ParametricExecution::exec(const char * query, bool canHaveErrors)
{
    MLOG_MESSAGE(Debug, "param exec: " << query);

    size_t size = values_.size();
    return conn_.exec(query, size, !size ? 0 : &values_[0], !size ? 0 : &lengths_[0], canHaveErrors);
}

Result ParametricExecution::exec(const std::string & query, bool canHaveErrors)
{
    return exec(query.c_str(), canHaveErrors);
}

void ParametricExecution::execVoid(const char * query, bool canHaveErrors)
{
    MLOG_MESSAGE(Debug, "param exec: " << query);

    size_t size = values_.size();
    conn_.execVoid(query, size, !size ? 0 : &values_[0], !size ? 0 : &lengths_[0], canHaveErrors);
}

void ParametricExecution::execVoid(const std::string & query, bool canHaveErrors)
{
    execVoid(query.c_str(), canHaveErrors);
}

size_t ParametricExecution::size() const
{
    return values_.size();
}

void ParametricExecution::addParam(boost::int64_t value)
{
    longInts_.push_back(mstd::hton(value));
    addImpl(&longInts_.back());
}

void ParametricExecution::addParam(boost::int32_t value)
{
    ints_.push_back(mstd::hton(value));
    addImpl(&ints_.back());
}

void ParametricExecution::addParam(boost::int16_t value)
{
    smallInts_.push_back(mstd::hton(value));
    addImpl(&smallInts_.back());
}

void ParametricExecution::addParam(const char *value, size_t length)
{
    values_.push_back(value);
    lengths_.push_back(length);
}

void ParametricExecution::addArray(const std::pair<const char*, const char*> * value, size_t size)
{
    typedef const std::pair<const char*, const char*> * iterator;

    iterator end = value + size;
    size_t bufferSize = 20 + 4 * size;

    for(iterator i = value; i != end; ++i)
        bufferSize += i->second - i->first;
    
    char * temp = new char[bufferSize]; // TODO
    buffers_.push_back(temp);
    char * out = temp;

    write4(out, 1);
    write4(out, 1);
    write4(out, psql::oidText);
    write4(out, size);
    write4(out, 1);

    for(; value != end; ++value)
    {
        size_t len = value->second - value->first;
        write4(out, len);
        memcpy(out, value->first, len);
        out += len;
    }

    addParam(temp, out - temp);
}

void ParametricExecution::addArray(const std::string * value, size_t size)
{
    typedef const std::string * iterator;

    iterator end = value + size;
    size_t bufferSize = 20 + 4 * size;

    for(iterator i = value; i != end; ++i)
        bufferSize += i->length();

    char * temp = new char[bufferSize]; // TODO
    buffers_.push_back(temp);
    char * out = temp;

    write4(out, 1);
    write4(out, 1);
    write4(out, psql::oidText);
    write4(out, size);
    write4(out, 1);

    for(; value != end; ++value)
    {
        size_t len = value->length();
        write4(out, len);
        memcpy(out, value->c_str(), len);
        out += len;
    }

    addParam(temp, out - temp);
}

void ParametricExecution::addArray(const boost::int64_t * value, size_t size)
{
    char * temp = new char[size * 12 + 20]; // TODO
    buffers_.push_back(temp);
    char * out = temp;

    write4(out, 1);
    write4(out, 1);
    write4(out, psql::oidInt64);
    write4(out, size);
    write4(out, 1);

    for(const boost::int64_t * end = value + size; value != end; ++value)
    {
        write4(out, 8);
        write8(out, *value);
    }

    addParam(temp, out - temp);
}

void ParametricExecution::addArray(const boost::int32_t * value, size_t size)
{
    char * temp = new char[size * 8 + 20]; // TODO
    buffers_.push_back(temp);
    char * out = temp;

    write4(out, 1);
    write4(out, 1);
    write4(out, psql::oidInt32);
    write4(out, size);
    write4(out, 1);

    for(const boost::int32_t * end = value + size; value != end; ++value)
    {
        write4(out, 4);
        write4(out, *value);
    }

    addParam(temp, out - temp);
}

void ParametricExecution::addArray(const boost::int16_t * value, size_t size)
{
    char * temp = new char[size * 6 + 20]; // TODO
    buffers_.push_back(temp);
    char * out = temp;

    write4(out, 1);
    write4(out, 1);
    write4(out, psql::oidInt16);
    write4(out, size);
    write4(out, 1);

    for(const boost::int16_t * end = value + size; value != end; ++value)
    {
        write4(out, 2);
        write2(out, *value);
    }

    addParam(temp, out - temp);
}

////////////////////////////////////////////////////////////////////////////////
// class Result
////////////////////////////////////////////////////////////////////////////////

Result::Result(PGresult * value)
    : value_(value), width_(0) {}

Result::Result()
    : value_(0), width_(0) {}

Result::~Result()
{
    if(value_)
        PQclear(value_);
}

bool Result::success() const
{
    ExecStatusType status = PQresultStatus(value_);
    return status == PGRES_COMMAND_OK || status == PGRES_TUPLES_OK;
}

ExecStatusType Result::status() const
{
    return PQresultStatus(value_);
}

const char * Result::error() const
{
    return PQresultErrorMessage(value_);
}

size_t Result::size() const
{
    return PQntuples(value_);
}

Result::reference Result::at(size_t index) const
{
    return ResultRowRef(value_, index, columns());
}

Oid Result::type(size_t index) const
{
    return PQftype(value_, index);
}

size_t Result::columns() const
{
    if(!width_)
        width_ = PQnfields(value_);
    return width_;
}

Result::reference Result::operator[](size_t index) const
{
    return at(index);
}

////////////////////////////////////////////////////////////////////////////////
// class ResultRowRef
////////////////////////////////////////////////////////////////////////////////

ResultRowRef::ResultRowRef(PGresult *result, size_t index, size_t size)
    : result_(result), index_(index), size_(size) {}

const void * ResultRowRef::raw(size_t index) const
{
    return PQgetvalue(result_, index_, index);
}

boost::int16_t ResultRowRef::asInt16(size_t index) const
{
    const void * data = PQgetvalue(result_, index_, index);
    Oid oid = PQftype(result_, index);
    if(oid != oidInt16)
        BOOST_THROW_EXCEPTION(InvalidTypeException() << OidInfo(oid) << ExpectedOidInfo(oidInt16));
    return mstd::ntoh(*static_cast<const boost::int16_t*>(data));
}

boost::int32_t ResultRowRef::asInt32(size_t index) const
{
    const void * data = PQgetvalue(result_, index_, index);
    Oid oid = PQftype(result_, index);
    if(oid != oidInt32)
        BOOST_THROW_EXCEPTION(InvalidTypeException() << OidInfo(oid) << ExpectedOidInfo(oidInt32));
    return mstd::ntoh(*static_cast<const boost::int32_t*>(data));
}

boost::int64_t ResultRowRef::asInt64(size_t index) const
{
    const void * data = PQgetvalue(result_, index_, index);
    Oid oid = PQftype(result_, index);
    if(oid != oidInt64)
        BOOST_THROW_EXCEPTION(InvalidTypeException() << OidInfo(oid) << ExpectedOidInfo(oidInt64));
    return mstd::ntoh(*static_cast<const boost::int64_t*>(data));
}

const char * ResultRowRef::asCString(size_t index) const
{
    const void * data = PQgetvalue(result_, index_, index);
    Oid oid = PQftype(result_, index);
    if(oid != oidText && oid != oidVarChar)
        BOOST_THROW_EXCEPTION(InvalidTypeException() << OidInfo(oid) << ExpectedOidInfo(oidText));
    return static_cast<const char*>(data);
}

ByteArray ResultRowRef::asArray(size_t index) const
{
    const void * data = PQgetvalue(result_, index_, index);
    Oid oid = PQftype(result_, index);
    if(oid != oidByteArray)
        BOOST_THROW_EXCEPTION(InvalidTypeException() << OidInfo(oid) << ExpectedOidInfo(oidByteArray));
    return ByteArray(PQgetlength(result_, index_, index), static_cast<const char*>(data));
}

size_t ResultRowRef::length(size_t index) const
{
    return PQgetlength(result_, index_, index);
}

bool ResultRowRef::null(size_t index) const
{
    return PQgetisnull(result_, index_, index) != 0;
}

size_t ResultRowRef::size() const
{
    return size_;
}

}
