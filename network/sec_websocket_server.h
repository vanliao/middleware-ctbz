#ifndef SEC_WEBSOCKET_SERVER_H
#define SEC_WEBSOCKET_SERVER_H

#include "websocket_server.h"

namespace network {

class SecWebsocketServer : public WebsocketServer
{
public:
    SecWebsocketServer(const std::string serverIP, const int serverPort);
    virtual ~SecWebsocketServer(void);
    void setCAFile(const bool verifyPeer,
                   const std::string &caFilePath,
                   const std::string &certFilePath,
                   const std::string &keyFilePath);
    bool listen(void);
    bool accept(unsigned int &connID);

private:
    bool verifyCA;
    std::string caFile;
    std::string certFile;
    std::string privateKeyFile;
};

}

#endif // SEC_WEBSOCKET_SERVER_H
