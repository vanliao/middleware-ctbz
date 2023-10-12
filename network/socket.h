#ifndef SOCKET_H
#define SOCKET_H

#include "ssl_common.h"
#include "end_point.h"

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
    SSL *sslHandle;
    SSL_CTX * sslCtx;
};

}
#endif // SOCKET_H
