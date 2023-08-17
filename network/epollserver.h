#ifndef EPOLLSERVER_H
#define EPOLLSERVER_H
#include "tcpserver.h"
#include "udpserver.h"

namespace network {

class EPollServerIF
{
public:
    EPollServerIF(){};
    virtual ~EPollServerIF(){};
    virtual void connectNotify(unsigned int connID) = 0;
    virtual void recvNotify(unsigned int connID, std::string &buf) = 0;
    virtual void closeNotify(unsigned int connID) = 0;
};

class EPollServer
{
public:
    enum ServerType
    {
        UDP,
        TCP
    };

    EPollServer(const std::string &serverIP, const int serverPort, const ServerType type);
    virtual ~EPollServer(void);
    bool start(EPollServerIF &obj);
    void stop(void);

private:
    bool startTcpSvr(EPollServerIF &obj);
    bool startUdpSvr(EPollServerIF &/*obj*/);

protected:
    int epollFd;
    ServerType svrType;
    std::shared_ptr<Socket> sock;
    volatile bool finish;
};

}
#endif // EPOLLSERVER_H
