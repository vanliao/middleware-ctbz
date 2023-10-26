#ifndef WSSECCLIENTDEMO_H
#define WSSECCLIENTDEMO_H

#include "common_communicator.h"

namespace service {

class WSSecClientDemo : public Communicator
{
public:
    WSSecClientDemo(const std::string &serverIP, const int serverPort);
    virtual ~WSSecClientDemo();
    void connectNotify(unsigned int connID) override;
    void recvNotify(unsigned int connID, std::string &buf) override;
    void closeNotify(unsigned int connID) override;
    void eventtNotify(const int event) override;
    void initExternEvent(void) override;
    void setSSLCaFile(const bool verifyPeer,
                      const std::string &caFile,
                      const std::string &certFile,
                      const std::string &keyFile);

private:
    void procEvent(void);
    void procDevResult(dev::Dev::ProcResult pr, dev::EndPoint *ep,
                       std::shared_ptr<msg::Msg> &reqMsg, std::shared_ptr<msg::Msg> &rspMsg);
};

}
#endif // WSSECCLIENTDEMO_H
