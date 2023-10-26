#ifndef WSSECSERVERDEMO_H
#define WSSECSERVERDEMO_H

#include "common_server.h"

namespace service {

class WSSecServerDemo : public Server
{
public:
    WSSecServerDemo(const std::string &serverIP, const int serverPort);
    virtual ~WSSecServerDemo();
    void connectNotify(unsigned int connID) override;
    void recvNotify(unsigned int connID, std::string &buf) override;
    void closeNotify(unsigned int connID) override;
    void eventtNotify(const int event) override;
    void initExternEvent(void) override;
    void setSSLCAFile(const bool verifyPeer,
                      const std::string &caFilePath,
                      const std::string &certFilePath,
                      const std::string &keyFilePath);

private:
    void procEvent(void);
    void procDevResult(dev::Dev::ProcResult pr, dev::EndPoint *ep,
                       std::shared_ptr<msg::Msg> &reqMsg, std::shared_ptr<msg::Msg> &rspMsg);
};

}
#endif // WSSECSERVERDEMO_H
