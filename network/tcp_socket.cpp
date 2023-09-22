#include <memory.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include "tcp_socket.h"
#include "tinylog.h"

namespace network {

TcpSocket::TcpSocket(): Socket(SOCK_STREAM)
{
    return;
}

TcpSocket::TcpSocket(const int sockFd): Socket(sockFd, SOCK_STREAM)
{
    return;
}

TcpSocket::~TcpSocket()
{
    return;
}

bool TcpSocket::bind()
{
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &servaddr.sin_addr);
    servaddr.sin_port = htons(port);

    int ret = ::bind(fd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr));
    if (-1 == ret)
    {
        log_error("bind socket failed:" << strerror(errno));
        return false;
    }

    return true;
}

bool TcpSocket::listen()
{
    int ret = ::listen(fd, 1024);
    if (-1 == ret)
    {
        log_error("listen socket failed:" << strerror(errno));
        return false;
    }

    return true;
}

bool TcpSocket::connect()
{
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &servaddr.sin_addr);
    servaddr.sin_port = htons(port);

    int ret = ::connect(fd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr));
    if (-1 == ret)
    {
        return false;
    }

    return true;
}

bool TcpSocket::accept(int &newSock, std::string &ipAddr, int &port)
{
    socklen_t len = sizeof(struct sockaddr);
    struct sockaddr_in peerAddr;
    int ret = ::accept(fd, (struct sockaddr *)&peerAddr, &len);
    if (-1 == ret)
    {
        log_error("accpet socket failed:" << strerror(errno));
        return false;
    }
    newSock = ret;
    log_debug("create fd " << newSock);

    char buff[64];
    ::inet_ntop(AF_INET,(const void *)&peerAddr.sin_addr,buff, sizeof(buff));
    ipAddr = buff;
    port = ntohs(peerAddr.sin_port);

    return true;
}

bool TcpSocket::recv(std::string &buf)
{
    char buffer[8192];
    while (1)
    {
        int ret = ::recv(fd, buffer, sizeof(buffer), 0);
        if (0 > ret)
        {
            if(EAGAIN == errno)
            {
                log_debug("tcp socket recv eagain");
                break;
            }
            log_error("recv failed:" << strerror(errno))
            return false;
        }
        else
        {
            buf.append(buffer, ret);
            if ((unsigned int)ret < sizeof(buffer))
            {
                /* 接收的数据长度小于缓冲区长度说明接收完了 */
                break;
            }
        }
    }


    return true;
}

bool TcpSocket::send(std::string &buf)
{
    std::string::size_type len = buf.length();
    int ret = ::send(fd, buf.c_str(), len, 0);
    if (0 > ret)
    {
        log_error("send failed:" << strerror(errno))
        return false;
    }

    buf.erase(0, ret);

    if ((unsigned int)ret != len)
    {
        return false;
    }

    return true;
}

}
