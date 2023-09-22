#include <sstream>
#include "udp_server.h"
#include "api.h"
#include "tinylog.h"

namespace network {

UdpServer::UdpServer(const std::string servrIP, const int serverPort): UdpSocket()
{
    ip = servrIP;
    port = serverPort;
    return;
}

UdpServer::~UdpServer()
{
    return;
}

bool UdpServer::bind()
{
    return UdpSocket::bind();
}

bool UdpServer::connect()
{
    return false;
}

bool UdpServer::accept(std::string &buf, unsigned int &connID)
{
    std::string remoteAddr;
    int remotePort;
    bool ret = UdpSocket::recv(buf, remoteAddr, remotePort);

    std::stringstream ss;
    ss << remoteAddr << ":" << remotePort;
    auto it = clientAddrs.find(ss.str());
    if (clientAddrs.end() == it)
    {
        unsigned int cltID = api::getClientID();
        clientAddrs.emplace(ss.str(), cltID);
        std::shared_ptr<UdpClient> clt = std::make_shared<UdpClient>(fd, remoteAddr, remotePort);
        auto it = clients.insert(std::pair<unsigned int, std::shared_ptr<UdpClient> >(cltID, clt));
        if (it.second)
        {
            connID = cltID;
            clt->connID = cltID;
            log_debug("create connection success:" << connID);
        }

        ret = true;
    }
    else
    {
        connID = it->second;
        ret = false;
    }

    return ret;
}

void UdpServer::close(const int connID)
{
    auto it = clients.find(connID);
    if (clients.end() != it)
    {
        clients.erase(it);
        log_debug("remove connection:" << connID);

        std::stringstream ss;
        ss << it->second->ip << ":" << it->second->port;
        auto itAddr = clientAddrs.find(ss.str());
        if (clientAddrs.end() != itAddr)
        {
            clientAddrs.erase(itAddr);
        }
    }
    return;
}

UdpClient *network::UdpServer::getClient(const unsigned int connID)
{
    auto it = clients.find(connID);
    if (clients.end() != it)
    {
        return it->second.get();
    }

    return NULL;
}

UdpClient *network::UdpServer::getClient(void)
{
    auto it = clients.begin();
    if (clients.end() != it)
    {
        return it->second.get();
    }

    return NULL;
}

}
