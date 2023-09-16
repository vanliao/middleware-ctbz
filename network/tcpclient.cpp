#include <errno.h>
#include "tcpclient.h"
#include "tinylog.h"

namespace network {

TcpClient::TcpClient(const std::string serverIP, const int serverPort): TcpSocket()
{
    ip = serverIP;
    port = serverPort;
    isPeer = false;
    doing = Do::NONE;
    return;
}

TcpClient::TcpClient(const int clientFd)
{
    fd = clientFd;
    isPeer = true;
    setNonBlock();
    return;
}

TcpClient::~TcpClient()
{
    return;
}

bool TcpClient::connect()
{
    if (isPeer)
    {
        log_debug("connect failed:" << "peer can not connect to somewhere")
        return false;
    }

    bool ret = create();
    ret &= TcpSocket::connect();
    if (!ret)
    {
#ifndef MIPS
        if (EAGAIN == errno || EINPROGRESS == errno)
#endif
        {
            doing = CONNECTING;
        }
    }

    return ret;
}

}
