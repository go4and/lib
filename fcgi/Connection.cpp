/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
include "pch.h"

#include "Connection.h"

MLOG_DECLARE_LOGGER(fcgi_conn);

namespace fcgi {

#pragma pack(push)
#pragma pack(1)
struct FCGIRecord {
    uint8_t version;
    uint8_t type;
    RequestId requestId;
    uint16_t contentLength;
    uint8_t paddingLength;
    uint8_t reserved;
};

struct FCGIBeginRequestBody {
    uint16_t role;
    uint8_t flags;
    uint8_t reserved[5];
};

struct FCGIEndRequestBody {
    uint32_t appStatus;
    uint8_t protocolStatus;
    uint8_t reserved[3];
};
#pragma pack(pop)

namespace {

const size_t bufferSize = 0x10000;

const uint8_t FCGI_BEGIN_REQUEST = 1;
const uint8_t FCGI_ABORT_REQUEST = 2;
const uint8_t FCGI_END_REQUEST = 3;
const uint8_t FCGI_PARAMS = 4;
const uint8_t FCGI_STDIN = 5;
const uint8_t FCGI_STDOUT = 6;
const uint8_t FCGI_STDERR = 7;
const uint8_t FCGI_DATA = 8;
const uint8_t FCGI_GET_VALUES = 9;
const uint8_t FCGI_GET_VALUES_RESULT = 10;
const uint8_t FCGI_UNKNOWN_TYPE = 11;
const uint8_t FCGI_MAXTYPE = FCGI_UNKNOWN_TYPE;

const uint8_t FCGI_RESPONDER  = 1;
const uint8_t FCGI_AUTHORIZER = 2;
const uint8_t FCGI_FILTER     = 3;

const uint8_t FCGI_REQUEST_COMPLETE = 0;
const uint8_t FCGI_CANT_MPX_CONN    = 1;
const uint8_t FCGI_OVERLOADED       = 2;
const uint8_t FCGI_UNKNOWN_ROLE     = 3;

const uint8_t FCGI_KEEP_CONN = 1;

size_t readLen(nexus::PacketReader & reader)
{
    size_t len = reader.read<uint8_t>();
    if(len & 0x80)
    {
        reader.revert(1);
        len = mstd::ntoh(reader.read<uint32_t>()) & 0x7fffffff;
    }
    return len;
}

}

Connection::Connection(const nexus::isocket & socket, const RequestHandler & requestHandler)
    : socket_(socket), requestHandler_(requestHandler), pos_(0)
{
}

Connection::~Connection()
{
    MLOG_MESSAGE(Debug, "~Connection()");
}

void Connection::send(const char * begin, const char * end)
{
    MLOG_MESSAGE(Debug, "send(" << requestId_ << ", " << mlog::dump(begin, end) << ")");

    const char * prefix = "Content-Type: text/plain\r\n\r\n";
    size_t prefixLen = strlen(prefix);
    size_t len = end - begin + prefixLen;
    size_t alen = (len + 7) / 8 * 8;
    RequestId reqId = mstd::hton(requestId_);

    output_.resize(0x1000); //sizeof(FCGIRecord) + ahlen + sizeof(FCGIRecord) + alen + sizeof(FCGIRecord) + sizeof(FCGIRecord) + sizeof(FCGIEndRequestBody));

    char * out = &output_[0];

    FCGIRecord * rec = mstd::pointer_cast<FCGIRecord*>(out);
    rec->version = 1;
    rec->type = FCGI_STDOUT;
    rec->requestId = reqId;
    rec->contentLength = mstd::hton<uint16_t>(len);
    rec->paddingLength = alen - len;
    rec->reserved = 0;
    out += sizeof(*rec);
    memcpy(out, prefix, prefixLen);
    out += prefixLen;
    memcpy(out, begin, end - begin);
    out += alen - prefixLen;

    rec = mstd::pointer_cast<FCGIRecord*>(out);
    rec->version = 1;
    rec->type = FCGI_STDOUT;
    rec->requestId = reqId;
    rec->contentLength = mstd::hton<uint16_t>(0);
    rec->paddingLength = 0;
    rec->reserved = 0;
    out += sizeof(*rec);

    rec = mstd::pointer_cast<FCGIRecord*>(out);
    rec->version = 1;
    rec->type = FCGI_END_REQUEST;
    rec->requestId = reqId;
    rec->contentLength = mstd::hton<uint16_t>(sizeof(FCGIEndRequestBody));
    rec->paddingLength = 0;
    rec->reserved = 0;
    out += sizeof(*rec);
    FCGIEndRequestBody * body = mstd::pointer_cast<FCGIEndRequestBody*>(out);
    body->appStatus = mstd::hton<uint32_t>(0);
    body->protocolStatus = FCGI_REQUEST_COMPLETE;
    out += sizeof(*body);

    output_.resize(out - &output_[0]);
    async_write(**socket_, boost::asio::buffer(output_), bindWrite(ptr()));
}

void Connection::startRead()
{
    MLOG_MESSAGE(Debug, "startRead()");

    if(buffer_.size() - pos_ < 0x10000)
    {
        buffer_.resize(pos_ + bufferSize);
        buffer_.resize(buffer_.capacity());
    }
    (*socket_)->async_read_some(boost::asio::buffer(&buffer_[pos_], buffer_.size() - pos_), bindRead(ptr()));
}

void Connection::handleRead(const boost::system::error_code & ec, size_t bt, const ConnectionPtr & ptr)
{
    MLOG_MESSAGE(Debug, "handleRead(" << ec << ')');

    if(!ec)
    {
        pos_ += bt;
        if(processRecords())
            startRead();
    }
}

bool Connection::processRecords()
{
    MLOG_MESSAGE(Debug, "processRecords()");

    while(pos_ >= sizeof(FCGIRecord))
    {
        FCGIRecord * rec = mstd::pointer_cast<FCGIRecord*>(&buffer_[0]);
        size_t recordLen = sizeof(*rec) + mstd::ntoh(rec->contentLength) + rec->paddingLength;
        if(pos_ >= recordLen)
        {
            if(!processRecord(*rec))
                return false;
            memcpy(&buffer_[0], &buffer_[0] + recordLen, pos_ - recordLen);
            pos_ -= recordLen;
        } else
            MLOG_MESSAGE(Debug, "Non full record: " << recordLen);
    }
    return true;
}

bool Connection::processRecord(const FCGIRecord & rec)
{
    MLOG_MESSAGE(Debug, "processRecord(type: " << static_cast<int>(rec.type) << ", len: " << mstd::ntoh(rec.contentLength) << 
                        ", request: " << mstd::ntoh(rec.requestId) << ", padding: " << rec.paddingLength << ")");

    char * begin = &buffer_[0] + sizeof(rec);
    switch(rec.type) {
    case FCGI_BEGIN_REQUEST:
        {
            FCGIBeginRequestBody * body = mstd::pointer_cast<FCGIBeginRequestBody*>(begin);
            keepAlive_ = (body->flags & FCGI_KEEP_CONN) != 0;
            stream_.clear();
        }
        break;
    case FCGI_PARAMS:
        {
            if(rec.contentLength)
                stream_.insert(stream_.end(), begin, begin + mstd::ntoh(rec.contentLength));
            else {
                processParams();
                stream_.clear();
            }
        }
        break;
    case FCGI_STDIN:
        {
            if(rec.contentLength)
                stream_.insert(stream_.end(), begin, begin + mstd::ntoh(rec.contentLength));
            else {
                RequestPtr request(new Request);
                requestId_ = mstd::ntoh(rec.requestId);
                request->params.swap(params_);
                request->body = stream_.empty() ? nexus::Buffer::blank() : nexus::Buffer(&stream_[0], stream_.size());
                stream_.clear();
                (*socket_)->get_io_service().post(boost::bind(requestHandler_, request, ptr()));
                if(!keepAlive_)
                    return false;
            }
        }
        break;
    default:
        MLOG_MESSAGE(Warning, "Unexpected type: " << static_cast<int>(rec.type));
        break;
    }
    return true;
}

void Connection::processParams()
{
    MLOG_MESSAGE(Debug, "processParams(" << stream_.size() << ")");
    MLOG_MESSAGE(Debug, "processParams(" << mlog::dump(stream_) << ")");

    nexus::PacketReader reader(stream_);
    while(reader.left())
    {
        size_t nameLen = readLen(reader);
        size_t valueLen = readLen(reader);
        if(reader.raw() <= reader.end() && nameLen + valueLen <= reader.left())
        {
            std::string name(reader.raw(), reader.raw() + nameLen);
            reader.skip(nameLen);
            std::string value(reader.raw(), reader.raw() + valueLen);
            reader.skip(valueLen);
            params_.insert(Params::value_type(name, value));
            MLOG_MESSAGE(Debug, "param: " << name << ", value: " << value);
        } else {
            MLOG_MESSAGE(Error, "left: " << reader.left() << ", nameLen: " << nameLen << ", valueLen: " << valueLen);
            break;
        }
    }
}

void Connection::handleWrite(const boost::system::error_code & ec, size_t bytes, const ConnectionPtr & conn)
{
    MLOG_MESSAGE(Debug, "handleWrite(" << ec << ')');
}

}
