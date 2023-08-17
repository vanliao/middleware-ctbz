#ifndef UDPSOCKET_H
#define UDPSOCKET_H

#include "socket.h"

namespace network {

class UdpSocket : public Socket
{
public:
    UdpSocket();
    virtual ~UdpSocket();
    bool bind(void);
    bool connect(void);
    bool recv(std::string &buf, std::string &remoteAddr, int &remotePort);
    bool send(const std::string &buf, const std::string &remoteAddr, const int &remotePort);
    bool recv(std::string &buf);
    bool send(const std::string &buf);
};

}

#endif // UDPSOCKET_H
