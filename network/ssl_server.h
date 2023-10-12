#ifndef SSLSERVER_H
#define SSLSERVER_H

#include "tcp_server.h"

namespace network {

class SSLServer : public TcpServer
{
public:
    SSLServer(const std::string serverIP, const int serverPort);
    virtual ~SSLServer(void);
    void setCAFile(const bool verifyPeer,
                   const std::string &caFilePath,
                   const std::string &certFilePath,
                   const std::string &keyFilePath);
    bool listen(void);
    bool accept(unsigned int &connID);

private:
    bool verifyPeerCA(SSL *sslHandle);

private:
    bool verifyCA;
    std::string caFile;
    std::string certFile;
    std::string privateKeyFile;
};

}

#endif // SSLSERVER_H
