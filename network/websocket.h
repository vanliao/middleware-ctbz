#ifndef WEBSOCKET_H
#define WEBSOCKET_H
#include <memory>

namespace network {

class WebSocket
{
public:
    WebSocket() {};
    virtual ~WebSocket() {};

public:
    enum OpCode
    {
        CONT = 0,
        TEXT = 1,
        BINARY = 2,
        CLOSE = 8,
        PING = 9,
        PONG = 10,
    };

    enum CloseStatus
    {
        CLOSE_NONE,
        CLOSE_RECV,
        CLOSE_SEND,
        CLOSE_FORCE,
        CLOSE_COMPLETE,
    };
};

}
#endif // WEBSOCKET_H
