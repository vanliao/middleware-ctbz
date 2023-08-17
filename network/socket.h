#ifndef SOCKET_H
#define SOCKET_H

#include "cgi_dev.h"

namespace network {

class Socket: public dev::EndPoint
{
public:
    Socket(const int sockType);
    Socket(const int sockFd, const int sockType);
    virtual ~Socket();
    bool create(void);
    void destroy(void);
    bool setNonBlock(void);

public:
    int fd;
    int type;
    int port;
    std::string ip;
};

}
#endif // SOCKET_H
