#pragma once

#include <deque>
#include <exception>
#include <memory>
#include <string>
#include <vector>
#include <iosfwd>

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/logic/tribool.hpp>

#include <mstd/cstdint.hpp>
#include <mstd/exception.hpp>
#include <mstd/hton.hpp>
#include <mstd/move.hpp>

typedef unsigned int Oid;
typedef struct pg_conn PGconn;
typedef struct pg_result PGresult;

namespace psql {

const Oid oidByteArray =   17;
const Oid oidInt64     =   20;
const Oid oidInt16     =   21;
const Oid oidInt32     =   23;
const Oid oidText      =   25;
const Oid oidVarChar   = 1043;
const Oid oidTimestamp = 1114;

class Result;

class ConnStatusTag;
typedef boost::error_info<ConnStatusTag, int> ConnStatus;

struct ByteArray {
    int len;
    const char * data;

    ByteArray(int len, const char * data)
        : len(len), data(data) {}
};

class ResultRowRef {
public:
    size_t size() const;

    size_t length(size_t index) const;
    bool null(size_t index) const;

    const void * raw(size_t index) const;
    
    boost::int16_t asInt16(size_t index) const;
    boost::int32_t asInt32(size_t index) const;
    boost::int64_t asInt64(size_t index) const;
    const char * asCString(size_t index) const;
    ByteArray asArray(size_t index) const;
    boost::posix_time::ptime asTime(size_t index) const;
    
    template<class T>
    typename boost::enable_if<boost::is_same<T, boost::int16_t>, T>::type
    as(size_t index) const {
        return asInt16(index);
    }

    template<class T>
    typename boost::enable_if<boost::is_same<T, boost::int32_t>, T>::type
    as(size_t index) const {
        return asInt32(index);
    }

    template<class T>
    typename boost::enable_if<boost::is_same<T, boost::int64_t>, T>::type
    as(size_t index) const {
        return asInt64(index);
    }

    template<class T>
    typename boost::enable_if<boost::is_same<T, const char*>, T>::type
    as(size_t index) const {
        return asCString(index);
    }

    template<class T>
    typename boost::enable_if<boost::is_same<T, ByteArray>, T>::type
    as(size_t index) const {
        return asArray(index);
    }


    template<class T>
    typename boost::enable_if<boost::is_same<T, boost::posix_time::ptime>, T>::type
    as(size_t index) const {
        return asTime(index);
    }
private:
    ResultRowRef(PGresult * result, size_t index, size_t size);

    PGresult * result_;
    size_t index_;
    size_t size_;

    friend class Result;
};

class Result {
public:
    typedef ResultRowRef value_type;
    typedef ResultRowRef reference;

    ~Result();

    size_t size() const;
    reference at(size_t index) const;
    reference operator[](size_t index) const;

    size_t columns() const;
    Oid type(size_t index) const;

    operator mstd::move_t<Result>()
    {
        return move();
    }
    
    mstd::move_t<Result> move()
    {
        mstd::move_t<Result> x(*this);
        return x;
    }

    Result();

    Result(mstd::move_t<Result> src)
        : value_(src->value_), width_(src->width_)
    {
        src->value_ = 0;
    }

    void operator=(mstd::move_t<Result> src)
    {
        Result temp(src);
        swap(temp);
    }

    void swap(Result & rhs)
    {
        std::swap(value_, rhs.value_);
        std::swap(width_, rhs.width_);
    }

    typedef size_t Result::*unspecified_bool_type;

    operator unspecified_bool_type() const
    {
        return value_ ? &Result::width_ : 0;
    }
private:
    void operator=(Result&);
    Result(Result&);

    Result(PGresult * value);

    bool success() const;
    const char * error() const;
    int status() const;

    PGresult * value_;
    mutable size_t width_;

    friend class Connection;
    friend class ParametricExecution;
};

const size_t copyDataBufferSize = 2048; 

struct CopyData {
    char * pos;
    char buffer[copyDataBufferSize];
    
    inline ptrdiff_t left() const
    {
        return buffer + copyDataBufferSize - pos;
    }
    
    inline void begin()
    {
        pos = buffer;
    }
};

class ParametricExecution;

class PGConnHolder : private boost::noncopyable {
public:
    PGConnHolder(const std::string & options);
    ~PGConnHolder();

    PGconn * conn();
private:
    PGconn * conn_;
};

class Connection : private PGConnHolder {
public:
    Connection(const std::string & options);
    ~Connection();

    Result exec(const char * query, const ParametricExecution & pe, bool canHaveErrors = false);

    inline Result exec(const std::string & query, const ParametricExecution & pe, bool canHaveErrors)
    {
        return exec(query.c_str(), pe, canHaveErrors);
    }
    
    Result exec(const char * query, bool canHaveErrors = false);

    inline Result exec(const std::string & query, bool canHaveErrors = false)
    {
        return exec(query.c_str(), canHaveErrors);
    }

    void execVoid(const char * query, bool canHaveErrors = false);

    inline void execVoid(const std::string & query, bool canHaveErrors = false)
    {
        execVoid(query.c_str(), canHaveErrors);
    }

    void copyBegin(const char * query, bool canHaveErrors = false);

    inline void copyBegin(const std::string & query, bool canHaveErrors = false)
    {
        copyBegin(query.c_str(), canHaveErrors);
    }

    void copyStartRow(int16_t count);
    void copyPutInt16(int16_t value);
    void copyPutInt32(int32_t value);
    void copyPutInt64(int64_t value);
    void copyPut(const char * value, size_t len);

    Result copyEnd();
private:
    void copyFlushBuffer();
    void copySendBuffer(const char * begin, int len);
    int wait(bool reading);
    void checkResult(const char * query, Result & result, bool canHaveErrors, const boost::posix_time::ptime & start);

    boost::scoped_ptr<CopyData> copyData_;
    std::vector<const char *> values_;
    std::vector<int> lengths_;
    std::vector<int> formats_;
};

enum IsolationLevel {
    ilDefault,
    ilSerializable,
    ilReadCommitted,
};

class Transaction : private boost::noncopyable {
public:
    Transaction(Connection & conn, IsolationLevel level = ilDefault);
    ~Transaction();
    
    void commit();
    void rollback();
private:
    Connection & conn_;
    boost::tribool commited_;
};

const size_t PEChunkSize = 0x80;

class PEChunk {
public:
    PEChunk()
        : next_(0)
    {
    }

    ~PEChunk()
    {
        delete next_;
    }

    void clear()
    {
        delete next_;
        next_ = 0;
    }

    void grow()
    {
        BOOST_ASSERT(!next_);
        next_ = new PEChunk;
    }

    inline char * begin() { return buffer_; }
    inline char * end() { return begin() + PEChunkSize; }
    inline PEChunk * next() { return next_; }

    inline const char * begin() const { return buffer_; }
    inline const char * end() const { return begin() + PEChunkSize; }
    inline const PEChunk * next() const { return next_; }
private:
    PEChunk * next_;
    char buffer_[PEChunkSize];
};

class ParametricExecution : private boost::noncopyable {
public:
    explicit ParametricExecution(Connection & conn);
    ~ParametricExecution();
    
    Result exec(const char * query, bool canHaveErrors = false);
    Result exec(const std::string & query, bool canHaveErrors = false);

    void execVoid(const char * query, bool canHaveErrors = false);
    void execVoid(const std::string & query, bool canHaveErrors = false);
    
    void addParam(const char * value, size_t length);

    void addParam(boost::int64_t value)
    {
        addImpl(mstd::hton(value));
    }

    void addParam(boost::int32_t value)
    {
        addImpl(mstd::hton(value));
    }

    void addParam(boost::int16_t value)
    {
        addImpl(mstd::hton(value));
    }

    template<class T>
    typename boost::enable_if<boost::is_same<T, bool>, void>::type
    addParam(T value)
    {
        addImpl(static_cast<uint8_t>(value));
    }

    void addParam(const boost::posix_time::ptime & value);

    void addStringReference(const std::string & str)
    {
        addParam(str.c_str(), str.length());
    }

    template<class T>
    void addArray(const std::vector<T> & value)
    {
        addArray(value.empty() ? 0 : &value[0], value.size());
    }

    void addArray(const std::pair<const char*, const char*> * value, size_t count);
    void addArray(const std::string * value, size_t count);
    void addArray(const boost::int64_t * value, size_t count);
    void addArray(const boost::int32_t * value, size_t count);
    void addArray(const boost::int16_t * value, size_t count);
    void clear();

    template<class T>
    void add(T t)
    {
        addParam(t);
    }
private:
    void fill(std::vector<const char*> & values, std::vector<int> & lengths) const; 

    inline void ensure(size_t len)
    {
        if(static_cast<size_t>(tail_->end() - out_) < len)
        {
            if(out_ != tail_->end())
                *out_ = 0xff;
            tail_->grow();
            tail_ = tail_->next();
            out_ = tail_->begin(); 
        }
    }

    template<class T>
    void addImpl(const T & t)
    {
        ensure(sizeof(t) + 1);
        *out_++ = sizeof(t);
        memcpy(out_, &t, sizeof(t));
        out_ += sizeof(t);
    }

    char * allocBuffer(size_t size);
    void freeExtraBuffers();

    Connection & conn_;
    PEChunk head_;
    PEChunk * tail_;
    char * out_;
    
    void * extraBuffers_;
    void ** extraBuffersTail_;
    
    friend class Connection;
};

inline std::ostream & operator<<(std::ostream & out, const ByteArray & value)
{
    out << '[' << value.len << ']';
    return out;
}

inline Result move(mstd::move_t<Result> t)
{
    return Result(t);
}

class PSQLTag;
typedef mstd::own_exception<PSQLTag> Exception;
class ConnectionTag;
typedef mstd::own_exception<ConnectionTag, Exception> ConnectionException;
class ExecTag;
typedef mstd::own_exception<ExecTag, Exception> ExecException;
class UnknownTypeTag;
typedef mstd::own_exception<UnknownTypeTag, Exception> UnknownTypeException;
class InvalidTypeTag;
typedef mstd::own_exception<InvalidTypeTag, Exception> InvalidTypeException;
class CopyFailedTag;
typedef mstd::own_exception<CopyFailedTag, Exception> CopyFailedException;

class OidTag;
typedef boost::error_info<OidTag, Oid> OidInfo;
class ExpectedOidTag;
typedef boost::error_info<ExpectedOidTag, Oid> ExpectedOidInfo;

}
