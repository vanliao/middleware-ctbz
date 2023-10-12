#ifndef EPOLL_COMMUNICATOR_H
#define EPOLL_COMMUNICATOR_H

#include <map>
#include <vector>
#include "tcp_client.h"
#include "udp_client.h"
#include "ssl_client.h"
#include "epoll_server.h"

namespace network {

class EpollCommunicatorIF;

class EpollCommunicator
{
public:
    enum ServerType
    {
        UDP,
        TCP,
        SSL,
        WS,
    };
    EpollCommunicator(const std::string &serverIP, const int serverPort, const ServerType svrtype);
    virtual ~EpollCommunicator();
    bool start(EpollCommunicatorIF &obj);
    void stop(void);
    bool connect(void);
    void disconnect(const int connID);
    bool sendWS(const int connID, const std::string &buf, const network::WebSocket::OpCode wsOpCode);
    network::TcpClient *getTcpClient(const int connID);
    network::UdpClient *getUdpClient(const int connID);
    void setSSLCAFile(const bool verifyPeer,
                      const std::string &caFilePath,
                      const std::string &certFilePath,
                      const std::string &keyFilePath);

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
    bool verifyCA;
    std::string caFile;
    std::string certFile;
    std::string privateKeyFile;
};

class EpollCommunicatorIF: public EpollServerIF{};

}

#endif // EPOLL_COMMUNICATOR_H
