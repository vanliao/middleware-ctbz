#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H

#include <map>
#include <vector>
#include "tcpclient.h"
#include "udpclient.h"
#include "epollserver.h"

namespace network {

class CommunicatorIF;

class Communicator
{
public:
    enum ServerType
    {
        UDP,
        TCP
    };
    Communicator(const std::string &serverIP, const int serverPort, const ServerType svrtype);
    virtual ~Communicator();
    bool open(void);
    bool start(CommunicatorIF &obj);
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
    bool startTcpSvr(CommunicatorIF &obj);
    bool startUdpSvr(CommunicatorIF &obj);
private:
    ServerType type;
    int port;
    std::string ip;
    int epollFd;
    bool finish;
    std::vector<int> eventFds;
    std::map<int, std::shared_ptr<network::Socket> > clients;
};

class CommunicatorIF: public network::EPollServerIF
{
public:
    CommunicatorIF(const std::string &serverIP, const int serverPort, const Communicator::ServerType svrtype):
        svr(serverIP, serverPort, svrtype){};
    virtual ~CommunicatorIF(){};

    virtual void eventtNotify(const int event) = 0;

protected:
    Communicator svr;
};

}

#endif // COMMUNICATOR_H
