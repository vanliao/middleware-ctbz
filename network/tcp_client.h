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
    POLL_RESULT pollIn(void);
    POLL_RESULT pollOut(void);

public:
    enum Status
    {
        NONE,
        CONNECTING,
        SSLCONNECTING,
        CONNECTED,
    };
    Status status;
    std::string sendBuf;
    std::string recvBuf;

public:
    POLL_RESULT connectAction(void);
    POLL_RESULT sendAction(void);
    std::function<POLL_RESULT(void)> pollOutAction;

protected:
    bool isPeer;
};

}
#endif // TCP_CLIENT_H
