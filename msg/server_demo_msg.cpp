#include "server_demo_msg.h"
#include "tinylog.h"

namespace msg {
namespace type {
namespace cgi {

void ServerDemoMsg::show(std::stringstream &ss, bool output)
{
    Msg::show(ss, false);
    ss << std::endl;
    ss << "Server Demo Msg" << std::endl;
    ss << "cmdType:" << cmdType << std::endl;
    ss << "code:" << code << std::endl;
    ss << "result:" << result << std::endl;

    if (output)
    {
        log_info(ss.str());
        ss.str("");
    }

    return;
}

}
}
}
