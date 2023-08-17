#ifndef SERVERDEMOMSG_H
#define SERVERDEMOMSG_H

#include "msg.h"

namespace msg {
namespace type {
namespace cgi {

class ServerDemoMsg : public Msg
{
public:
    ServerDemoMsg(){src = msg::src::SRC_CGI;type = msg::type::cgi::MSG_TEST;};
    ~ServerDemoMsg(){};
    int cmdType;
    int code;
    int result;
    void show(std::stringstream &ss, bool output = true);
};

}
}
}

#endif // SERVERDEMOMSG_H
