#ifndef SSLSERVER_H
#define SSLSERVER_H

#include "tcp_server.h"

namespace network {

class SSLServer : public TcpServer
{
public:
    SSLServer(const std::string serverIP, const int serverPort);
    virtual ~SSLServer(void);
    void setCertificateFile(const std::string &filePath);
    void setPrivateKeyFile(const std::string &filePath);
    bool listen(void);
    bool accept(unsigned int &connID);

private:
    std::string certFile;
    std::string privateKeyFile;
    SSL_CTX * sslCtx;
};

}

#endif // SSLSERVER_H
