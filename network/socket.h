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
    enum POLL_RESULT
    {
        POLL_RESULT_SUCCESS,
        POLL_RESULT_READ,
        POLL_RESULT_SEND,
        POLL_RESULT_HANDSHAKE,
        POLL_RESULT_CLOSE,
    };
    virtual POLL_RESULT pollIn();
    virtual POLL_RESULT pollOut();

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
