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
    enum Do
    {
        NONE,
        CONNECTING,
        SENDING
    };
    Do doing;

private:
    static unsigned int clientID;
    bool isPeer;
};

}
#endif // TCPCLIENT_H
