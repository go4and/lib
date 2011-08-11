#include "pch.h"

#include "SocialApi.h"

MLOG_DECLARE_LOGGER(social_api);

namespace nexus {

SocialRequest::SocialRequest(SocialNetwork type)
    : url_(type == sn_vkont ? "http://api.vkontakte.ru/api.php?" : "http://www.appsmail.ru/platform/api?")
{
}

void SocialRequest::addParam(const std::string & param, const std::string & value)
{
    str_ += param;
    str_ += '=';
    str_ += value;
    url_ += param;
    url_ += '=';
    url_ += value;
    url_ += '&';
}

void SocialRequest::addParam(const std::string & param, const char * value)
{
    str_ += param;
    str_ += '=';
    str_ += value;
    url_ += param;
    url_ += '=';
    url_ += value;
    url_ += '&';
}

void SocialRequest::addParam(const char * param, const std::string & value)
{
    str_ += param;
    str_ += '=';
    str_ += value;
    url_ += param;
    url_ += '=';
    url_ += value;
    url_ += '&';
}

void SocialRequest::addParam(const char * param, const char * value)
{
    str_ += param;
    str_ += '=';
    str_ += value;
    url_ += param;
    url_ += '=';
    url_ += value;
    url_ += '&';
}

const std::string & SocialRequest::url(const std::string & apiSecret)
{
    str_ += apiSecret;
    std::string sig = mcrypt::visualize(mcrypt::md5String(str_));
    url_ += "sig=";
    url_ += sig;

    return url_;
}

class ReadStream : public boost::noncopyable, public mstd::reference_counter<ReadStream> {
public:
    explicit ReadStream(boost::asio::io_service & ioService)
        : impl_(ioService) {}

    urdl::read_stream * operator->()
    {
        return &impl_;
    }

    urdl::read_stream & operator*()
    {
        return impl_;
    }
private:
    urdl::read_stream impl_;
};

typedef boost::intrusive_ptr<ReadStream> ReadStreamPtr;

template<bool strict>
class ReadHandler {
public:
    ReadHandler(const ReadStreamPtr & stream, const URLHandler & handler, const nexus::Buffer & buffer)
        : stream_(stream), handler_(handler), buffer_(buffer) {}

    void operator()(const boost::system::error_code & ec, size_t bytes)
    {
        if(!strict)
        {
            if(ec == boost::asio::error::eof)
            {
                buffer_.resize(bytes);
                handler_(boost::system::error_code(), buffer_);
                return;
            }
        }
        handler_(ec, buffer_);
    }
private:
    ReadStreamPtr stream_;
    URLHandler handler_;
    nexus::Buffer buffer_;
};

class OpenHandler {
public:
    OpenHandler(const ReadStreamPtr & stream, const URLHandler & handler)
        : stream_(stream), handler_(handler) {}

    void operator()(const boost::system::error_code & ec) const
    {
        if(!ec)
        {
            ReadStream & rs = *stream_;
            size_t len = rs->content_length();
            MLOG_MESSAGE(Debug, "open done: " << len);
            if(len == 0)
            {
                nexus::Buffer buf(0x1000);
                boost::asio::async_read(**stream_, boost::asio::buffer(buf.data(), buf.size()), ReadHandler<false>(stream_, handler_, buf));
/*                const std::string & headers = rs->headers();
                if(headers.find("\r\nTransfer-Encoding: chunked\r\n") != std::string::npos)
                {
                    MLOG_MESSAGE(Debug, "chunked");
                }*/
            } else if(len <= 0x100000)
            {
                nexus::Buffer buf(len);
                boost::asio::async_read(**stream_, boost::asio::buffer(buf.data(), buf.size()), ReadHandler<true>(stream_, handler_, buf));
            } else {
                boost::system::error_code ec = urdl::http::errc::make_error_code(urdl::http::errc::malformed_response_headers);
                nexus::Buffer buf;
                handler_(ec, buf);
            }
        } else {
            nexus::Buffer buf;
            handler_(ec, buf);
        }
    }
private:
    ReadStreamPtr stream_;
    URLHandler handler_;
};

void getUrl(boost::asio::io_service & ios, const std::string & url, const URLHandler & handler)
{
    ReadStreamPtr stream(new ReadStream(ios));
    (*stream)->async_open(url, OpenHandler(stream, handler));
}

}
