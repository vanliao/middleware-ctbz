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
    bool recv(std::string &buf);
    std::string creatWSHeader(const uint64_t payloadLen, const uint8_t opcode, const bool fin = true);
    void closeSend();

private:
    static uint32_t rol(uint32_t value, uint32_t bits);
    uint32_t sha1base64(uint8_t* in, uint64_t in_len, char* out);
    uint32_t handleHttpRequest(std::string &wsMsg);
    bool handleWSMsg(std::string &wsMsg);

public:
    bool isWSUpdated;
    WebSocket::CloseStatus closeStatus;
    std::string recvBuf;
    uint8_t wsOpCode;
    std::deque<std::string> sendBuf;
};

}
#endif // WEBSOCKETCLIENT_H
