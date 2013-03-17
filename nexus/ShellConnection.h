#pragma once

namespace nexus {

typedef boost::function<void(std::ostream&, const std::vector<std::string>&)> ShellConnectionListener;

class ShellConnection;
boost::weak_ptr<ShellConnection> startShellConnection(boost::asio::local::stream_protocol::socket & socket, const std::string & welcome, const ShellConnectionListener & listener);
boost::weak_ptr<ShellConnection> startShellConnection(boost::asio::ip::tcp::socket & socket, const std::string & welcome, const ShellConnectionListener & listener);
void stopShellConnection(const boost::weak_ptr<ShellConnection> & conn);

}
