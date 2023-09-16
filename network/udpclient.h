#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <deque>
#include "udpsocket.h"

namespace network {

class UdpClient : public UdpSocket
{
public:
    UdpClient(const std::string servrIP, const int serverPort);
    UdpClient(const int svrFd, const std::string peerIP, const int peerPort);
    virtual ~UdpClient();
    bool bind(void);
    bool connect(void);
    bool recv(std::string &buf);
    bool send(const std::string &buf);

public:
    enum Status
    {
        NONE,
        CONNECTING,
        CONNECTED,
    };
    Status status;
    std::deque<std::string> sendBuf;

private:
    bool isPeer;
};

}
#endif // UDPCLIENT_H
