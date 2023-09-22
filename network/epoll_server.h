#ifndef EPOLL_SERVER_H
#define EPOLL_SERVER_H
#include "tcp_server.h"
#include "udp_server.h"

namespace network {

class EpollServerIF
{
public:
    EpollServerIF(){};
    virtual ~EpollServerIF(){};
    virtual void connectNotify(unsigned int connID) = 0;
    virtual void recvNotify(unsigned int connID, std::string &buf) = 0;
    virtual void closeNotify(unsigned int connID) = 0;
};

class EpollServer
{
public:
    enum ServerType
    {
        UDP,
        TCP
    };

    EpollServer(const std::string &serverIP, const int serverPort, const ServerType type);
    virtual ~EpollServer(void);
    bool start(EpollServerIF &obj);
    void stop(void);

private:
    bool startTcpSvr(EpollServerIF &obj);
    bool startUdpSvr(EpollServerIF &obj);

protected:
    int epollFd;
    ServerType svrType;
    std::shared_ptr<Socket> sock;
    volatile bool finish;
};

}
#endif // EPOLL_SERVER_H
