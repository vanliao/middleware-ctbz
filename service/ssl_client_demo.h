#ifndef SSLCLIENTDEMO_H
#define SSLCLIENTDEMO_H

#include "common_communicator.h"

namespace service {

class SSLClientDemo : public Communicator
{
public:
    SSLClientDemo(const std::string &serverIP, const int serverPort);
    virtual ~SSLClientDemo();
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

#endif // SSLCLIENTDEMO_H
