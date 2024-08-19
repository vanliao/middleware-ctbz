#include <sys/types.h>
#include <sys/socket.h>
#include "tcp_server.h"
#include "tinylog.h"
#include "api.h"

namespace network {

TcpServer::TcpServer(const std::string serverIP, const int serverPort): TcpSocket()
{
    ip = serverIP;
    port = serverPort;
    return;
}

TcpServer::~TcpServer()
{
    return;
}

bool TcpServer::bind()
{
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    return TcpSocket::bind();
}

bool TcpServer::listen()
{
    return TcpSocket::listen();
}

bool TcpServer::accept(unsigned int &connID)
{
    int sock;
    std::string ip;
    int port;
    bool ret = TcpSocket::accept(sock, ip, port);
    if (!ret)
    {
        return false;
    }

    if (clients.end() != clients.find(sock))
    {
        log_error("socket[" << sock << "] has been used");
        return false;
    }

    std::shared_ptr<TcpClient> clt = std::make_shared<TcpClient>(sock);
    auto it = clients.insert(std::pair<unsigned int, std::shared_ptr<TcpClient> >(
                  api::getClientID(std::bind(&TcpServer::checkClientKey, this, std::placeholders::_1)), clt));
    if (it.second)
    {
        connID = it.first->first;
        clt->connID = connID;
        clt->ip = ip;
        clt->port = port;
        log_debug("create connection success:" << connID);
    }

    return it.second;
}

void TcpServer::close(const int connID)
{
    auto it = clients.find(connID);
    if (clients.end() != clients.find(connID))
    {
        clients.erase(it);
        log_debug("remove connection:" << connID);
    }
    return;
}

bool TcpServer::recv(std::string &/*buf*/)
{
    return false;
}

bool TcpServer::send(std::string &/*buf*/)
{
    return false;
}

TcpClient *TcpServer::getClient(const unsigned int connID)
{
    auto it = clients.find(connID);
    if (clients.end() != it)
    {
        return it->second.get();
    }

    return NULL;
}

TcpClient *TcpServer::getClient()
{
    auto it = clients.begin();
    if (clients.end() != it)
    {
        return it->second.get();
    }

    return NULL;
}

bool TcpServer::checkClientKey(unsigned int cltID)
{
    auto it = clients.find(cltID);
    if (clients.end() != it)
    {
        /* id 已存在 */
        return false;
    }

    return true;
}

}
