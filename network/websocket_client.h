#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <deque>
#include "tcp_client.h"
#include "websocket.h"

namespace network {

class WebsocketClient : public TcpClient
{
public:
    WebsocketClient(const std::string serverIP, const int serverPort);
    WebsocketClient(const int clientFd);
    virtual ~WebsocketClient();
    bool connectWS(void);
    bool sendPrepare(const std::string &buf, const uint8_t opcode);
    void closeSend();

public:
    POLL_RESULT pollIn(void);
    POLL_RESULT pollOut(void);
    POLL_RESULT connectAction(void);
    POLL_RESULT sendAction(void);
    POLL_RESULT handShakeAction1(void);
    POLL_RESULT handShakeAction2(void);
    POLL_RESULT recvAction(void);
    std::function<POLL_RESULT(void)> pollInAction;
    std::function<POLL_RESULT(void)> pollOutAction;

    std::string creatWSHeader(const uint64_t payloadLen, const uint8_t opcode, const bool fin = true);
    static uint32_t rol(uint32_t value, uint32_t bits);
    uint32_t sha1base64(uint8_t* in, uint64_t in_len, char* out);
    bool handleHttpRequest(std::string &wsMsg);
    bool handleHttpReply(std::string &wsMsg);
    POLL_RESULT handleWSMsg(std::string &wsMsg);

public:
    bool isWSUpdated;
    WebSocket::CloseStatus closeStatus;
    uint8_t wsOpCode;
    std::deque<std::string> sendBuf;
    std::string readyWSFrame;
    std::string partialWSFrame;

protected:
    std::string uri;
    std::string host;
    std::string origin;
    std::string protocol;
    std::string extensions;
};

}
#endif // WEBSOCKETCLIENT_H
