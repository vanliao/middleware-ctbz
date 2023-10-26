#ifndef SEC_WEBSOCKET_CLIENT_H
#define SEC_WEBSOCKET_CLIENT_H

#include <deque>
#include "websocket_client.h"

namespace network {

class SecWebsocketClient : public WebsocketClient
{
public:
    SecWebsocketClient(const std::string &serverIP,
                       const int serverPort,
                       const bool verifyPeer,
                       const std::string &caFilePath,
                       const std::string &certFilePath,
                       const std::string &keyFilePath);
    SecWebsocketClient(const int clientFd);
    virtual ~SecWebsocketClient(void);
    POLL_RESULT pollIn(void);
    POLL_RESULT pollOut(void);

private:
    bool SSLconnect(void);
    bool recv(std::string &buf);
    bool send(std::string &buf);
    POLL_RESULT connectTCPAction();
    POLL_RESULT connectWSAction();
    POLL_RESULT sendAction();
    POLL_RESULT wsHandShakeAction1(void);
    POLL_RESULT wsHandShakeAction2(void);
    POLL_RESULT recvAction(void);

private:
    bool verifyCA;
    std::string caFile;
    std::string certFile;
    std::string privateKeyFile;
};

}

#endif // SEC_WEBSOCKET_CLIENT_H
