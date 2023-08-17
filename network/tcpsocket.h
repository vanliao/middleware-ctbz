#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include "socket.h"

namespace network {

class TcpSocket: public Socket
{
public:
    TcpSocket();
    TcpSocket(const int sockFd);
    virtual ~TcpSocket();
    bool bind(void);
    bool listen(void);
    bool connect(void);
    bool accept(int &newSock, std::string &ipAddr, int &port);
    bool recv(std::string &buf);
    bool send(const std::string &buf);
};

}
#endif // TCPSOCKET_H
