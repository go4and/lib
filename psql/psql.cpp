#if defined(_MSC_VER)
#pragma warning(disable: 4996)

#include <WinSock2.h>
#endif

#include <boost/date_time/posix_time/posix_time_io.hpp>

#include <libpq-fe.h>

#include <mstd/hton.hpp>
#include <mstd/null.hpp>
#include <mstd/pointer_cast.hpp>

#include <mlog/Logging.h>

#include <mlog/Dumper.h>

#include "psql.h"

MLOG_DECLARE_LOGGER(psql);

int	psqlPQntuples(const PGresult *res);

namespace psql {

namespace {

boost::posix_time::ptime timeStart(boost::gregorian::date(2000, boost::date_time::Jan, 1));

void write2(char *& pos, int16_t value)
{
    *mstd::pointer_cast<int16_t*>(pos) = mstd::hton(value);
    pos += 2;
}

void write4(char *& pos, int32_t value)
{
    *mstd::pointer_cast<int32_t*>(pos) = mstd::hton(value);
    pos += 4;
}

void write8(char *& pos, int64_t value)
{
    *mstd::pointer_cast<int64_t*>(pos) = mstd::hton(value);
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

int Connection::wait(bool reading)
{
    int socket = PQsocket(conn());

    fd_set sockets;
    FD_ZERO(&sockets);
    FD_SET(socket, &sockets);

    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;

    if(reading)
        return select(socket + 1, &sockets, 0, 0, &timeout);
    else
        return select(socket + 1, 0, &sockets, 0, &timeout);
}

Connection::Connection(const std::string & cmd)
    : PGConnHolder(cmd)
{
    if(!conn())
        BOOST_THROW_EXCEPTION(ConnectionException());
    if(PQstatus(conn()) == CONNECTION_BAD)
        BOOST_THROW_EXCEPTION(ConnectionException() << mstd::error_message(PQerrorMessage(conn())));

    PostgresPollingStatusType status = PGRES_POLLING_WRITING;
    while(status != PGRES_POLLING_OK)
    {
        int result = wait(status == PGRES_POLLING_READING);

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
        MLOG_MESSAGE_EX(!canHaveErrors ? mlog::llError : mlog::llNotice, "exec failed: " << result.status() << ", msg: " << result.error() << ", query: " << query);
        BOOST_THROW_EXCEPTION(ExecException() << mstd::error_message(result.error()));
    }
}

Result Connection::exec(const char * query, const ParametricExecution & pe, bool canHaveErrors)
{
    MLOG_DEBUG("exec(" << query << ")");

    values_.clear();
    lengths_.clear();
    pe.fill(values_, lengths_);
    size_t size = values_.size();

    if(size > formats_.size())
    {
        size_t oldSize = formats_.size();
        formats_.resize(size);
        std::fill_n(formats_.begin() + oldSize, size - oldSize, 1);
    }

    boost::posix_time::ptime start = boost::posix_time::microsec_clock::universal_time();
    Result result(PQexecParams(conn(), query, size, 0, &values_[0], &lengths_[0], &formats_[0], 1));
    checkResult(query, result, canHaveErrors, start);

    MLOG_MESSAGE(Debug, "exec succeeded");

    return result;
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

void Connection::execVoid(const char * query, bool canHaveErrors)
{
    MLOG_MESSAGE(Debug, "execVoid(" << query << ")");

    boost::posix_time::ptime start = boost::posix_time::microsec_clock::universal_time();
    Result result(PQexecParams(conn(), query, 0, 0, 0, 0, 0, 0));
    checkResult(query, result, canHaveErrors, start);

    MLOG_MESSAGE(Debug, "exec succeeded");
}

void Connection::copyBegin(const char * query, bool canHaveErrors)
{
    MLOG_DEBUG("copyBegin(" << query << ", " << canHaveErrors << ")");
    
    boost::posix_time::ptime start = boost::posix_time::microsec_clock::universal_time();
    Result result(PQexecParams(conn(), query, 0, 0, 0, 0, 0, 0));
    checkResult(query, result, canHaveErrors, start);
    
    if(!copyData_)
        copyData_.reset(new CopyData);
    copyData_->begin();

    static const char prefix[] = "PGCOPY\n\xff\r\n\0\0\0\0\0\0\0\0\0";
    static const size_t prefixLen = sizeof(prefix) - 1;
    memcpy(copyData_->pos, prefix, prefixLen);
    copyData_->pos += prefixLen;
}

void Connection::copyStartRow(int16_t columns)
{
    if(copyData_->left() < 2)
        copyFlushBuffer();
    write2(copyData_->pos, columns);
}

void Connection::copySendBuffer(const char * begin, int len)
{
    // MLOG_DEBUG("copySendBuffer(" << mlog::dump(begin, len) << ")");
    int res;
    while(!(res = PQputCopyData(conn(), begin, len)))
        wait(false);
    if(res < 0)
        BOOST_THROW_EXCEPTION(CopyFailedException());
}

void Connection::copyFlushBuffer()
{
    char * begin = copyData_->buffer;
    int len = copyData_->pos - begin;
    if(len)
    {
        copySendBuffer(begin, len);
        copyData_->pos = begin;
    }
}

void Connection::copyPutInt16(int16_t value)
{
    if(copyData_->left() < 6)
        copyFlushBuffer();
    write4(copyData_->pos, 2);
    write2(copyData_->pos, value);
}

void Connection::copyPutInt32(int32_t value)
{
    if(copyData_->left() < 8)
        copyFlushBuffer();
    write4(copyData_->pos, 4);
    write4(copyData_->pos, value);
}

void Connection::copyPutInt64(int64_t value)
{
    if(copyData_->left() < 12)
        copyFlushBuffer();
    write4(copyData_->pos, 8);
    write8(copyData_->pos, value);
}

void Connection::copyPut(const char * value, size_t len)
{
    if(copyData_->left() < 4)
        copyFlushBuffer();
    write4(copyData_->pos, len);
    size_t left = copyData_->left();
    if(left >= len)
    {
        memcpy(copyData_->pos, value, len);
        copyData_->pos += len;        
    } else if(left + copyDataBufferSize - 0x10 > len)
    {        
        memcpy(copyData_->pos, value, left);
        copyData_->pos += left;
        copyFlushBuffer();

        value += left;
        len -= left;
        memcpy(copyData_->pos, value, len);
        copyData_->pos += len;
    } else {
        copyFlushBuffer();
        copySendBuffer(value, len);
    }
}

Result Connection::copyEnd()
{
    if(copyData_->left() < 2)
        copyFlushBuffer();
    write2(copyData_->pos, -1);
    copyFlushBuffer();
    int res;
    while(!(res = PQputCopyEnd(conn(), 0)))
        wait(false);
    if(res < 0)
        BOOST_THROW_EXCEPTION(CopyFailedException());

    boost::posix_time::ptime start = boost::posix_time::microsec_clock::universal_time();
    Result result(PQgetResult(conn()));
    checkResult("<END COPY>", result, false, start);
    
    MLOG_DEBUG("copyEnd - ok");
    
    return move(result);
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
    : conn_(conn), tail_(&head_), out_(tail_->begin()), extraBuffers_(0), extraBuffersTail_(&extraBuffers_)
{
}

void ParametricExecution::freeExtraBuffers()
{
    void * p = extraBuffers_;
    while(p)
    {
        void * n = *static_cast<void**>(p);
        delete [] static_cast<char*>(p);
        p = n;
    }
}

ParametricExecution::~ParametricExecution()
{
}

void ParametricExecution::clear()
{
    head_.clear();
    tail_ = &head_;
    out_ = tail_->begin();
    freeExtraBuffers();
    extraBuffersTail_ = &extraBuffers_;
}

Result ParametricExecution::exec(const char * query, bool canHaveErrors)
{
    MLOG_MESSAGE(Debug, "param exec: " << query);

    return conn_.exec(query, *this, canHaveErrors);
}

Result ParametricExecution::exec(const std::string & query, bool canHaveErrors)
{
    return exec(query.c_str(), canHaveErrors);
}

void ParametricExecution::execVoid(const char * query, bool canHaveErrors)
{
    MLOG_MESSAGE(Debug, "param exec: " << query);

    conn_.exec(query, *this, canHaveErrors);
}

void ParametricExecution::execVoid(const std::string & query, bool canHaveErrors)
{
    execVoid(query.c_str(), canHaveErrors);
}

void ParametricExecution::addParam(const boost::posix_time::ptime & value)
{
    addParam((value - timeStart).total_microseconds());
}

void ParametricExecution::addParam(const char *value, size_t length)
{
    ensure(4 + sizeof(value));
    *out_++ = 0x80 | ((length & 0x3f000000) >> 24);
    memcpy(out_, &length, 3);
    out_ += 3;
    memcpy(out_, &value, sizeof(value));
    out_ += sizeof(value);
}

void ParametricExecution::fill(std::vector<const char*> & values, std::vector<int> & lengths) const
{
    const PEChunk * c = &head_;
    while(c)
    {
        const char * inp = c->begin(), * end = c == tail_ ? out_ :  c->end();
        while(inp != end)
        {
            unsigned char x = *inp;
            ++inp;
            if(x == 0xff)
                break;
            else if(x & 0x80)
            {
                size_t length = 0;
                memcpy(&length, inp, 3);
                inp += 3;
                length |= (x & 0x3f) << 24;
                const char * address;
                memcpy(&address, inp, sizeof(address));
                inp += sizeof(address);
                values.push_back(address);
                lengths.push_back(length);
            } else {
                values.push_back(inp);
                lengths.push_back(x);
                inp += x;
            }
        }
        c = c->next();
    }
}

char * ParametricExecution::allocBuffer(size_t size)
{
    char * result = new char[size + sizeof(void*)];
    *extraBuffersTail_ = result;
    extraBuffersTail_ = mstd::pointer_cast<void**>(result);
    *extraBuffersTail_ = 0;
    return result + sizeof(void*);
}

void ParametricExecution::addArray(const std::pair<const char*, const char*> * value, size_t size)
{
    typedef const std::pair<const char*, const char*> * iterator;

    iterator end = value + size;
    size_t bufferSize = 20 + 4 * size;

    for(iterator i = value; i != end; ++i)
        bufferSize += i->second - i->first;
    
    char * temp = allocBuffer(bufferSize);
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

    char * temp = allocBuffer(bufferSize);
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
    char * temp = allocBuffer(size * 12 + 20);
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
    char * temp = allocBuffer(size * 8 + 20);
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
    char * temp = allocBuffer(size * 6 + 20);
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
    return status == PGRES_COMMAND_OK || status == PGRES_TUPLES_OK || status == PGRES_COPY_IN || status == PGRES_COPY_OUT;
}

int Result::status() const
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

boost::posix_time::ptime ResultRowRef::asTime(size_t index) const
{
    const void * data = PQgetvalue(result_, index_, index);
    Oid oid = PQftype(result_, index);
    if(oid != oidTimestamp)
        BOOST_THROW_EXCEPTION(InvalidTypeException() << OidInfo(oid) << ExpectedOidInfo(oidTimestamp));
    int64_t value = mstd::ntoh(*static_cast<const boost::int64_t*>(data));
    int64_t mul = (boost::posix_time::microseconds::traits_type::res_adjust() / 1000000);
    return timeStart + boost::posix_time::time_duration(0, 0, 0, value * mul);
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
