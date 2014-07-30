/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include "pch.h"

#include "Handler.h"
#include "ShellConnection.h"

MLOG_DECLARE_LOGGER(nexus_shell_connection);

namespace nexus {

class ShellConnection;

typedef boost::shared_ptr<ShellConnection> ShellConnectionPtr;

class ShellConnection {
public:
    virtual void cancel() = 0;
    virtual void start(const ShellConnectionPtr & self) = 0;

    virtual ~ShellConnection()
    {
    }
private:
};

template<class Socket>
class ShellConnectionImpl : public ShellConnection {
public:
    explicit ShellConnectionImpl(Socket & socket, const std::string & welcome, const ShellConnectionListener & listener)
        : socket_(boost::move(socket)), strand_(socket_.get_io_service()), welcome_(welcome), listener_(listener),
          readBuffer_(0x100)
    {
    }

    void start(const ShellConnectionPtr & self)
    {
        socket_.non_blocking(true);
        requestCommand(self);
    }

    void cancel()
    {
        strand_.dispatch(std::bind(&ShellConnectionImpl::close, this));
    }
private:
    void startWrite(const ShellConnectionPtr & self)
    {
        socket_.async_write_some(boost::asio::null_buffers(), strand_.wrap(bindWrite(self)));
    }

    void close()
    {
        boost::system::error_code ec;
        socket_.close(ec);
    }

    void handleWrite(const boost::system::error_code & ec, size_t size, const ShellConnectionPtr & self)
    {
        MLOG_DEBUG("handleWrite(" << ec << ", " << size << ")");
    
        if(!ec)
        {
            if(writeBuffer_.size() && !syncWrite(self))
                close();
        } else {
            MLOG_NOTICE("wait write failed: " << ec << ", " << ec.message());
            close();
        }
    }

    bool syncWrite(const ShellConnectionPtr & self)
    {
        MLOG_DEBUG("syncWrite()");

        boost::system::error_code ec;
        size_t len = socket_.write_some(writeBuffer_.data(), ec);
        if(ec && ec != boost::asio::error::would_block)
        {
            MLOG_NOTICE("write failed: " << ec << ", " << ec.message());
            return false;
        }
        writeBuffer_.consume(len);
        if(writeBuffer_.size())
            startWrite(self);
        return true;
    }

    void add(boost::asio::streambuf & buf, const std::string & line, bool addEol)
    {
        buf.sputn(line.c_str(), line.size());
        if(addEol)
            buf.sputn("\n\r", 2);
    }

    bool send(const std::string & line, bool addEol, const ShellConnectionPtr & self)
    {
        bool wasEmpty = !writeBuffer_.size();
        add(writeBuffer_, line, addEol);
        if(wasEmpty)
            return syncWrite(self);
        return true;
    }

    void requestCommand(const ShellConnectionPtr & self)
    {
        send(welcome_, false, self);
        startRead(self);
    }

    void startRead(const ShellConnectionPtr & self)
    {
        async_read_until(socket_, readBuffer_, '\n', strand_.wrap(bindRead(self)));
    }

    void handleRead(const boost::system::error_code & ec, size_t size, const ShellConnectionPtr & self)
    {
        MLOG_DEBUG("handleRead(" << ec << ", " << size << ")");

        buffer_.resize(0x1000);
        if(!ec && size < buffer_.size())
        {
            readBuffer_.sgetn(&buffer_[0], size);

            bool hasR = size >= 2 && buffer_[size - 2] == '\r';
            std::vector<std::string> args;
            mstd::split_args(args, buffer_.begin(), buffer_.begin() + size - (hasR ? 2 : 1));

            std::ostringstream out;

            try {
                listener_(out, args);
            } catch(boost::exception & exc) {
                out << "Failed: " << mstd::out_exception(exc) << std::endl;
            } catch(std::exception & exc) {
                out << "Failed: " << mstd::out_exception(exc) << std::endl;
            } catch(...) {
                out << "Unknown failure" << std::endl;
            }

            std::string response = out.str();
            if(hasR)
            {
                std::string temp;
                const char * p = response.c_str();
                const char * i = p;
                while(*i)
                {
                    if(*i == '\n')
                    {
                        ++i;
                        temp.insert(temp.end(), p, i);
                        temp += '\r';
                        p = i;
                    } else
                        ++i;
                }
                if(p != i)
                    temp.insert(temp.end(), p, i);
                temp.swap(response);
            }
            
            if(send(response, false, self))
                requestCommand(self);
            else
                close();
        } else
            close();
    }
private:
    Socket socket_;
    boost::asio::strand strand_;
    std::string welcome_;
    ShellConnectionListener listener_;
    boost::asio::streambuf readBuffer_;
    boost::asio::streambuf writeBuffer_;
    std::vector<char> buffer_;
    
    NEXUS_DECLARE_HANDLER_EX(Read, ShellConnectionImpl, true, 2);
    NEXUS_DECLARE_HANDLER_EX(Write, ShellConnectionImpl, true, 2);
};

boost::weak_ptr<ShellConnection> startShellConnection(boost::asio::local::stream_protocol::socket & socket, const std::string & welcome, const ShellConnectionListener & listener)
{
    ShellConnectionPtr conn(new ShellConnectionImpl<boost::asio::local::stream_protocol::socket>(socket, welcome, listener));
    conn->start(conn);
    return conn;
}

boost::weak_ptr<ShellConnection> startShellConnection(boost::asio::ip::tcp::socket & socket, const std::string & welcome, const ShellConnectionListener & listener)
{
    ShellConnectionPtr conn(new ShellConnectionImpl<boost::asio::ip::tcp::socket>(socket, welcome, listener));
    conn->start(conn);
    return conn;
}

void stopShellConnection(const boost::weak_ptr<ShellConnection> & conn)
{
    auto sconn = conn.lock();
    if(sconn)
        sconn->cancel();
}

}
