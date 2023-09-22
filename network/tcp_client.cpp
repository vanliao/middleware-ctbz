#include <errno.h>
#include "tcp_client.h"
#include "tinylog.h"

namespace network {

TcpClient::TcpClient(const std::string serverIP, const int serverPort): TcpSocket()
{
    ip = serverIP;
    port = serverPort;
    isPeer = false;
    status = Status::NONE;
    sendBuf = "";
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
            status = CONNECTING;
        }
    }

    return ret;
}

}
