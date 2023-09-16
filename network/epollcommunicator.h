#ifndef EPOLLCOMMUNICATOR_H
#define EPOLLCOMMUNICATOR_H

#include <map>
#include <vector>
#include "tcpclient.h"
#include "udpclient.h"
#include "epollserver.h"

namespace network {

class EpollCommunicatorIF;

class EpollCommunicator
{
public:
    enum ServerType
    {
        UDP,
        TCP
    };
    EpollCommunicator(const std::string &serverIP, const int serverPort, const ServerType svrtype);
    virtual ~EpollCommunicator();
    bool start(EpollCommunicatorIF &obj);
    void stop(void);
    bool connect(void);
    void disconnect(const int connID);
    network::TcpClient *getTcpClient(const int connID);
    network::UdpClient *getUdpClient(const int connID);

protected:
    bool startTcpSvr(EpollCommunicatorIF &obj);
    bool startUdpSvr(EpollCommunicatorIF &obj);

protected:
    ServerType type;
    int port;
    std::string ip;
    int epollFd;
    bool finish;
    std::map<int, std::shared_ptr<network::Socket> > clients;
};

class EpollCommunicatorIF: public EpollServerIF{};

}

#endif // EPOLLCOMMUNICATOR_H
