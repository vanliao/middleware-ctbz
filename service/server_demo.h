#ifndef SERVERDEMO_H
#define SERVERDEMO_H

#include "common_server.h"

namespace service {

class ServerDemo : public Server
{
public:
    ServerDemo(const std::string &serverIP, const int serverPort);
    virtual ~ServerDemo();
    void connectNotify(unsigned int connID) override;
    void recvNotify(unsigned int connID, std::string &buf) override;
    void closeNotify(unsigned int connID) override;
    void eventtNotify(const int event) override;
    void initExternEvent(void) override;

private:
    void procEvent(void);
    void procDevResult(dev::Dev::ProcResult pr, dev::EndPoint *ep,
                       std::shared_ptr<msg::Msg> &reqMsg, std::shared_ptr<msg::Msg> &rspMsg);
};

}
#endif // SERVERDEMO_H
