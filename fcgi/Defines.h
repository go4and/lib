#pragma  once

namespace fcgi {

typedef uint16_t RequestId;
typedef boost::unordered_map<std::string, std::string> Params;

class Connection;
typedef boost::intrusive_ptr<Connection> ConnectionPtr;

struct Request : public mstd::reference_counter<Request> {
    Params params;
    nexus::Buffer body;

    const std::string & param(const std::string & name) const
    {
        Params::const_iterator i = params.find(name);
        return i != params.end() ? i->second : mstd::default_instance<std::string>();
    }
};
typedef boost::intrusive_ptr<Request> RequestPtr;

typedef boost::function<void(const RequestPtr & request, const ConnectionPtr & conn)> RequestHandler;

}
