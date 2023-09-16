#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include "tcpsocket.h"

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
#endif // TCPCLIENT_H
