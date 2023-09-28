#include "websocket_server.h"
#include "websocket_client.h"
#include "tinylog.h"
#include "api.h"

namespace network {

WebsocketServer::WebsocketServer(const std::string serverIP, const int serverPort) :
    TcpServer(serverIP, serverPort)
{
    return;
}

WebsocketServer::~WebsocketServer()
{
    return;
}

bool WebsocketServer::accept(unsigned int &connID)
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

    std::shared_ptr<TcpClient> clt = std::make_shared<WebsocketClient>(sock);
    auto it = clients.insert(std::pair<unsigned int, std::shared_ptr<TcpClient> >(api::getClientID(), clt));
    if (it.second)
    {
        connID = it.first->first;
        clt->connID = connID;
        clt->ip = ip;
        clt->port = port;
        log_debug("ws create connection success:" << connID);
    }

    return it.second;
}

}
