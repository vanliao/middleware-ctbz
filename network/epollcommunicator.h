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
    bool open(void);
    bool start(EpollCommunicatorIF &obj);
    void stop(void);
    bool connect(void);
    void disconnect(const int connID);
    network::TcpClient *getTcpClient(const int connID);
    network::UdpClient *getUdpClient(const int connID);
    bool addExtenEvent(const int ev);
    bool delExtenEvent(const int ev);
    dev::EndPoint *getDev(const int connID);
    dev::EndPoint *getFirstDev(void);

private:
    bool startTcpSvr(EpollCommunicatorIF &obj);
    bool startUdpSvr(EpollCommunicatorIF &obj);

protected:
    ServerType type;
    int port;
    std::string ip;
    int epollFd;
    bool finish;
    std::vector<int> eventFds;
    std::map<int, std::shared_ptr<network::Socket> > clients;
};

class EpollCommunicatorIF: public network::EpollServerIF
{
public:
    EpollCommunicatorIF(const std::string &serverIP, const int serverPort, const EpollCommunicator::ServerType svrtype):
        svr(serverIP, serverPort, svrtype){};
    virtual ~EpollCommunicatorIF(){};

    virtual void eventtNotify(const int event) = 0;

protected:
    EpollCommunicator svr;
};

}

#endif // EPOLLCOMMUNICATOR_H
