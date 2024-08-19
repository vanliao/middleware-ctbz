#ifndef UDP_SERVER_H
#define UDP_SERVER_H

#include <map>
#include "udp_socket.h"
#include "udp_client.h"

namespace network {

class UdpServer : public UdpSocket
{
public:
    UdpServer(const std::string servrIP, const int serverPort);
    virtual ~UdpServer();
    bool bind(void);
    bool connect(void);
    bool accept(std::string &buf, unsigned int &connID);
    void close(const int connID);
    UdpClient *getClient(const unsigned int connID);
    UdpClient *getClient(void);
    bool checkClientKey(unsigned int cltID);
public:
    std::map<unsigned int, std::shared_ptr<UdpClient> > clients;
    std::map<std::string, unsigned int> clientAddrs;
};

}
#endif // UDP_SERVER_H
