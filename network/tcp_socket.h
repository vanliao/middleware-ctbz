#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include "socket.h"

namespace network {

class TcpSocket: public Socket
{
public:
    TcpSocket();
    TcpSocket(const int sockFd);
    virtual ~TcpSocket();
    virtual bool bind(void);
    virtual bool listen(void);
    virtual bool connect(void);
    virtual bool accept(int &newSock, std::string &ipAddr, int &port);
    virtual bool recv(std::string &buf);
    virtual bool send(std::string &buf);
};

}
#endif // TCP_SOCKET_H
