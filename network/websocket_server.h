#ifndef WEBSOCKETSERVER_H
#define WEBSOCKETSERVER_H

#include "tcp_server.h"
#include "websocket_client.h"

namespace network {

class WebsocketServer : public TcpServer
{
public:
    WebsocketServer(const std::string serverIP, const int serverPort);
    virtual ~WebsocketServer();
    bool accept(unsigned int &connID);
};

}
#endif // WEBSOCKETSERVER_H
