#ifndef EPOLL_SERVER_H
#define EPOLL_SERVER_H
#include "tcp_server.h"
#include "udp_server.h"
#include "websocket_server.h"
#include "sec_websocket_server.h"
#include "ssl_server.h"

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
        TCP,
        SSL,
        WS,
        WSS,
    };

    EpollServer(const std::string &serverIP, const int serverPort, const ServerType type);
    virtual ~EpollServer(void);
    bool start(EpollServerIF &obj);
    void stop(void);
    void setSSLCAFile(const bool verifyPeer,
                      const std::string &caFilePath,
                      const std::string &certFilePath,
                      const std::string &keyFilePath);

private:
    bool startTcpSvr(EpollServerIF &obj);
    bool startWSSvr(EpollServerIF &obj);
    bool startUdpSvr(EpollServerIF &obj);

protected:
    int epollFd;
    ServerType svrType;
    std::shared_ptr<Socket> sock;
    volatile bool finish;
};

}
#endif // EPOLL_SERVER_H
