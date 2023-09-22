#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include "tcp_socket.h"

namespace network {

class TcpClient: public TcpSocket
{
public:
    TcpClient(const std::string serverIP, const int serverPort);
    TcpClient(const int clientFd);
    virtual ~TcpClient(void);
    bool connect();

public:
    enum Status
    {
        NONE,
        CONNECTING,
        CONNECTED,
    };
    Status status;
    std::string sendBuf;

private:
    bool isPeer;
};

}
#endif // TCP_CLIENT_H
