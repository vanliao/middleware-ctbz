#include <functional>
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
    recvBuf = "";
    pollOutAction = std::bind(&TcpClient::connectAction, this);
    return;
}

TcpClient::TcpClient(const int clientFd)
{
    fd = clientFd;
    isPeer = true;
    status = Status::NONE;
    sendBuf = "";
    recvBuf = "";
    setNonBlock();
    pollOutAction = std::bind(&TcpClient::sendAction, this);
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
//    else
//    {
//        status = CONNECTED;
//    }

    return ret;
}

Socket::POLL_RESULT TcpClient::pollIn()
{
    if (likely(TcpSocket::recv(recvBuf)))
    {
        if (recvBuf.empty())
        {
            /* 接收成功 但接收缓存是空 则继续接收 */
            return POLL_RESULT_READ;
        }

        /* 接收完成 */
        return POLL_RESULT_SUCCESS;
    }

    /* 接收时遇到错误 关闭连接 */
    return POLL_RESULT_CLOSE;
}

Socket::POLL_RESULT TcpClient::pollOut()
{
    return pollOutAction();
}

Socket::POLL_RESULT TcpClient::connectAction()
{
    if (unlikely(CONNECTING != status))
    {
        log_error("invaild tcp status");
        /* 状态错误 关闭连接 */
        return POLL_RESULT_CLOSE;
    }

    status = CONNECTED;
    /* 之后收到写事件 执行数据发送流程 */
    pollOutAction = std::bind(&TcpClient::sendAction, this);

    return POLL_RESULT_HANDSHAKE;
}

Socket::POLL_RESULT TcpClient::sendAction()
{
    if (likely(TcpSocket::send(sendBuf)))
    {
        if (sendBuf.empty())
        {
            /* 发送完成 */
            return POLL_RESULT_SUCCESS;
        }

        /* 还有数据 之后继续发送 */
        return POLL_RESULT_SEND;
    }

    /* 发送时遇到错误 关闭连接 */
    return POLL_RESULT_CLOSE;
}

}
