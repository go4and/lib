#include "pch.h"

#include "icmp_header.hpp"

#include "Handler.h"

MLOG_DECLARE_LOGGER(nexus_traceroute);

using boost::asio::ip::icmp;

namespace nexus {

namespace {

static unsigned short getIdentifier()
{
#if defined(BOOST_WINDOWS)
    return static_cast<unsigned short>(::GetCurrentProcessId());
#else
    return static_cast<unsigned short>(::getpid());
#endif
}

class TraceRoute: public boost::noncopyable {
public:
    TraceRoute(const std::string & host)
        : resolver(io), socket(io, icmp::v4()), sequence_number(0)
    {
        icmp::resolver::query query(icmp::v4(), host, "");
        destination = *resolver.resolve(query);
    }
    
    ~TraceRoute()
    {
        socket.close();
    }

    void trace(std::ostream & out)
    {
        for(int ttl = 1; ttl < 31; ++ttl)
        {
            const boost::asio::ip::unicast::hops option( ttl );
            socket.set_option(option);

            boost::asio::ip::unicast::hops op;
            socket.get_option(op);
            if( ttl !=  op.value() )
            {
                out << "TTL not set properly. Should be " << ttl << " but was set to " << op.value() << '.' << std::endl;
                return;
            }

            out << ttl << ")\t";
            std::string ip;
            for(int t = 1; t != 4; ++t)
            {
                // Create an ICMP header for an echo request.
                icmp_header echo_request;
                echo_request.type(icmp_header::echo_request);
                echo_request.code(0);
                echo_request.identifier(getIdentifier());
                echo_request.sequence_number(++sequence_number);
                const std::string body("");
                compute_checksum(echo_request, body.begin(), body.end());

                // Encode the request packet.
                boost::asio::streambuf request_buffer;
                std::ostream os(&request_buffer);
                os << echo_request << body;

                boost::posix_time::ptime start = boost::posix_time::microsec_clock::universal_time();
                // Send the request.
                socket.send_to(request_buffer.data(), destination);

                // Recieve some data and parse it.
                std::vector<boost::uint8_t> data(64, 0);
                const std::size_t nr = socket.receive(boost::asio::buffer(data));
                if( nr < 16 )
                {
                    out << "To few bytes returned." << std::endl;
                    return;
                }
                boost::posix_time::ptime stop = boost::posix_time::microsec_clock::universal_time();
                if(ip.empty())
                {
                    char buf[0x20];
                    ip = mstd::itoa(data[12], buf);
                    ip += '.';
                    ip += mstd::itoa(data[13], buf);
                    ip += '.';
                    ip += mstd::itoa(data[14], buf);
                    ip += '.';
                    ip += mstd::itoa(data[15], buf);
                }
                out << (stop - start).total_milliseconds() << "\t";
            }
            out << ip << std::endl;
            if(boost::asio::ip::address_v4::from_string(ip) == destination.address())
                break;
         }
      }
private:
    static const int port = 33434;
    boost::asio::io_service io;
    boost::asio::ip::icmp::resolver resolver;
    boost::asio::ip::icmp::socket socket;
    unsigned short sequence_number;
    boost::asio::ip::icmp::endpoint destination;

};

class AsyncTraceRoute;
typedef boost::intrusive_ptr<AsyncTraceRoute> AsyncTraceRoutePtr;

class AsyncTraceRoute : public mstd::reference_counter<AsyncTraceRoute> {
public:
    AsyncTraceRoute(boost::asio::io_service & ios, const std::string & host, const std::function<void(const std::string &)> & listener)
        : resolver_(ios), socket_(ios, boost::asio::generic::datagram_protocol(AF_INET, IPPROTO_UDP)), host_(host), listener_(listener), sequenceNumber_(0)
    {
        if(!listener_)
            MLOG_ERROR("empty listener in traceroute");
    }

    ~AsyncTraceRoute()
    {
        socket_.close();
    }

    void start()
    {
        MLOG_DEBUG("start(), host = " << host_);

        boost::system::error_code ec;
        auto address = boost::asio::ip::address::from_string(host_);
        if(ec)
        {
            icmp::resolver::query query(icmp::v4(), host_, "");
            resolver_.async_resolve(query, std::bind(&AsyncTraceRoute::handleResolve, this, std::placeholders::_1, std::placeholders::_2, self()));
        } else {
            destination_ = boost::asio::ip::icmp::endpoint(address, 0);
            startPing(1, self());
        }
    }
private:
    AsyncTraceRoutePtr self()
    {
        return AsyncTraceRoutePtr(this);
    }

    void notify()
    {
        listener_(out_.str());
    }

    void handleResolve(const boost::system::error_code & ec, const boost::asio::ip::icmp::resolver::iterator & iterator, const AsyncTraceRoutePtr & self)
    {
        MLOG_DEBUG("handleResolve(" << ec << ", " << ec.message() << ")");

        if(!ec && iterator != boost::asio::ip::icmp::resolver::iterator())
        {
            auto dest = *iterator;
            // MLOG_DEBUG("dest: " << dest);
            startPing(1, self);
        } else {
            out_ << "Resolve '" << host_ << "' failed" << std::endl;
            notify();
        }
    }

    void startPing(int ttl, const AsyncTraceRoutePtr & self)
    {
        const boost::asio::ip::unicast::hops option(ttl);
        socket_.set_option(option);

        boost::asio::ip::unicast::hops op;
        socket_.get_option(op);
        if(ttl != op.value())
        {
            out_ << "TTL not set properly. Should be " << ttl << " but was set to " << op.value() << '.' << std::endl;
            notify();
        } else {
            out_ << ttl << ")\t";
            startSubping(ttl, 1, self);
        }
    }
    
    void startSubping(int ttl, int t, const AsyncTraceRoutePtr & self)
    {
        icmp_header echo_request;
        echo_request.type(icmp_header::echo_request);
        echo_request.code(0);
        echo_request.identifier(getIdentifier());
        echo_request.sequence_number(++sequenceNumber_);
        const std::string body("");
        compute_checksum(echo_request, body.begin(), body.end());

        // Encode the request packet.
        requestBuffer_ = boost::in_place();
        std::ostream os(&*requestBuffer_);
        os << echo_request << body;

        socket_.async_send_to(requestBuffer_->data(), destination_, bindSendTo(ttl, t, boost::posix_time::microsec_clock::universal_time(), self));
    }

    void handleSendTo(const boost::system::error_code & ec, size_t transferred, int ttl, int t, const boost::posix_time::ptime & start, const AsyncTraceRoutePtr & self)
    {
        MLOG_DEBUG("handleSendTo(" << ec << ", " << transferred << ", " << ttl << ", " << t << ", " << start << ")");

        if(!ec)
        {
            data_.resize(64);
            memset(&data_[0], 0, data_.size());
            socket_.async_receive(boost::asio::buffer(data_), bindReceive(ttl, t, start, self));
        } else {
            out_ << "send: " << ec.message() << '\t';
            nextStep(ttl, t, self);
        }
    }
    
    void handleReceive(const boost::system::error_code & ec, size_t transferred, int ttl, int t, const boost::posix_time::ptime & start, const AsyncTraceRoutePtr & self)
    {
        MLOG_DEBUG("handleReceive(" << ec << ", " << transferred << ", " << ttl << ", " << t << ", " << start << ")");

        if(!ec)
        {
            if(transferred < 16)
            {
                out_ << "short: " << transferred << '\t';
                nextStep(ttl, t, self);
            } else {
                boost::posix_time::ptime stop = boost::posix_time::microsec_clock::universal_time();
                out_ << (stop - start).total_milliseconds() << '\t';
                lastIp_ = boost::asio::generic::datagram_protocol::endpoint(&data_[12], 4);
                nextStep(ttl, t, self);
            }
        } else {
            out_ << "recv: " << ec.message() << '\t';
            nextStep(ttl, t, self);
        }
    }
    
    void nextStep(int ttl, int t, const AsyncTraceRoutePtr & self)
    {
        if(t >= 3)
        {
            if(ttl > 30 || lastIp_ == destination_)
                notify();
            else
                startPing(ttl + 1, self);
        } else
            startSubping(ttl, t + 1, self);
    }

    boost::asio::ip::icmp::resolver resolver_;
    boost::asio::generic::datagram_protocol::endpoint destination_;
    boost::asio::generic::datagram_protocol::socket socket_;
    boost::optional<boost::asio::streambuf> requestBuffer_;
    std::string host_;
    std::function<void(const std::string &)> listener_;
    std::ostringstream out_;
    unsigned short sequenceNumber_;
    boost::asio::generic::datagram_protocol::endpoint lastIp_;
    std::vector<uint8_t> data_;

    NEXUS_DECLARE_HANDLER(SendTo, AsyncTraceRoute, true);
    NEXUS_DECLARE_HANDLER(Receive, AsyncTraceRoute, true);
};

}

std::string makeTrace(const std::string & host)
{
    TraceRoute t(host);
    std::ostringstream out;
    t.trace(out);
    return out.str();
}

void makeTraceAsync(boost::asio::io_service & ios, const std::string & host, const std::function<void(const std::string &)> & listener)
{
    AsyncTraceRoutePtr tracer(new AsyncTraceRoute(ios, host, listener));
    tracer->start();
}

}
