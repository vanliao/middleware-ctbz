#include <memory.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>
#include "udp_socket.h"
#include "tinylog.h"

namespace network {

UdpSocket::UdpSocket() : Socket(SOCK_DGRAM)
{
    return;
}

UdpSocket::~UdpSocket()
{
    return;
}

bool UdpSocket::bind()
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

bool UdpSocket::connect()
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

bool UdpSocket::recv(std::string &buf, std::string &remoteAddr, int &remotePort)
{
    struct sockaddr_in raddr;
    memset(&raddr, 0, sizeof(raddr));
    socklen_t len = sizeof(raddr);

    buf = "";
    char buffer[8192];  //缓冲区长度根据需要修改
    while (1)
    {
        /* UDP必须一次性吧数据读取完,否则会丢弃 */
        int ret = ::recvfrom(fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&raddr, &len);
        if (0 > ret)
        {
            if(EAGAIN == errno)
            {
                log_debug("upd socket recv eagain");
                break;
            }
            log_error("recv failed:" << strerror(errno))
            return false;
        }
        else
        {
            if (buf.empty())
            {
                remoteAddr = inet_ntoa(raddr.sin_addr);
                remotePort = ntohs(raddr.sin_port);
            }

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

bool UdpSocket::send(const std::string &buf, const std::string &remoteAddr, const int &remotePort)
{
    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(remotePort);
    saddr.sin_addr.s_addr = inet_addr(remoteAddr.c_str());

    std::string::size_type len = buf.length();
    int ret = ::sendto(fd, buf.c_str(), len, 0, (struct sockaddr *)&saddr, sizeof(struct sockaddr));
    if (0 > ret)
    {
        log_error("send failed:" << strerror(errno))
        return false;
    }

    return true;
}

bool UdpSocket::recv(std::string &buf)
{
    struct sockaddr_in raddr;
    memset(&raddr, 0, sizeof(raddr));
    socklen_t len = sizeof(raddr);

    buf = "";
    char buffer[8192];
    while (1)
    {
        /* UDP必须一次性吧数据读取完,否则会丢弃 */
        int ret = ::recvfrom(fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&raddr, &len);
        log_debug("udp recv len:" << ret);
        if (0 > ret)
        {
            if(EAGAIN == errno)
            {
                log_debug("upd socket recv eagain");
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

bool UdpSocket::send(const std::string &buf)
{
    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = inet_addr(ip.c_str());

    std::string::size_type len = buf.length();
    int ret = ::sendto(fd, buf.c_str(), len, 0, (struct sockaddr *)&saddr, sizeof(struct sockaddr));
    if (0 > ret)
    {
        log_error("send failed:" << strerror(errno))
        return false;
    }

    return true;
}

}
