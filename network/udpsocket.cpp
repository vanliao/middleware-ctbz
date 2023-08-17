#include <memory.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>
#include "udpsocket.h"
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

    char buffer[8192];
    int ret = ::recvfrom(fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&raddr, &len);
    if (0 > ret)
    {
        log_error("recv failed:" << strerror(errno))
        return false;
    }
    remoteAddr = inet_ntoa(raddr.sin_addr);
    remotePort = ntohs(raddr.sin_port);

    buf.append(buffer, ret);

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

    char buffer[8192];
    int ret = ::recvfrom(fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&raddr, &len);
    if (0 > ret)
    {
        log_error("recv failed:" << strerror(errno))
        return false;
    }

    buf.append(buffer, ret);

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
