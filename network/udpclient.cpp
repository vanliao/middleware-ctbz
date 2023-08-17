#include "udpclient.h"

namespace network {

UdpClient::UdpClient(const std::string servrIP, const int serverPort): UdpSocket()
{
    ip = servrIP;
    port = serverPort;
    isPeer = false;
    return;
}

UdpClient::UdpClient(const int svrFd, const std::string peerIP, const int peerPort)
{
    fd = svrFd;
    ip = peerIP;
    port = peerPort;
    isPeer = true;
}

UdpClient::~UdpClient()
{
    if (isPeer)
    {
        fd = -1;
    }
    return;
}

bool UdpClient::bind()
{
    return false;
}

bool UdpClient::connect()
{
    if (isPeer)
    {
        return false;
    }
    if (!create())
    {
        return false;
    }

    return UdpSocket::connect();
}

bool UdpClient::recv(std::string &buf)
{
    return UdpSocket::recv(buf);
}

bool UdpClient::send(const std::string &buf)
{
    return UdpSocket::send(buf);
}

}
