#ifndef CLIENTDEMO_H
#define CLIENTDEMO_H

#include "common_communicator.h"

namespace service {

class ClientDemo : public Communicator
{
public:
    ClientDemo(const std::string &serverIP, const int serverPort);
    virtual ~ClientDemo();
    void connectNotify(unsigned int connID);
    void recvNotify(unsigned int connID, std::string &buf);
    void closeNotify(unsigned int connID);
    void eventtNotify(const int event);

private:
    void procEvent(void);
    void procDevResult(dev::Dev::ProcResult pr, dev::EndPoint *ep,
                       std::shared_ptr<msg::Msg> &reqMsg, std::shared_ptr<msg::Msg> &rspMsg);
};

}

#endif // CLIENTDEMO_H
