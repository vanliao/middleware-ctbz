#ifndef SSLCLIENT_H
#define SSLCLIENT_H

#include "tcp_client.h"

namespace network {

class SSLClient : public TcpClient
{
public:
    SSLClient(const std::string serverIP, const int serverPort);
    SSLClient(const int clientFd);
    virtual ~SSLClient(void);
    bool SSLconnect();
    bool recv(std::string &buf);
    bool send(std::string &buf);

private:
    SSL_CTX * sslCtx;
};

}

#endif // SSLCLIENT_H
