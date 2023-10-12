#ifndef SSLCLIENT_H
#define SSLCLIENT_H

#include "tcp_client.h"

namespace network {

class SSLClient : public TcpClient
{
public:
    SSLClient(const std::string &serverIP,
              const int serverPort,
              const bool verifyPeer,
              const std::string &caFilePath,
              const std::string &certFilePath,
              const std::string &keyFilePath);
    SSLClient(const int clientFd);
    virtual ~SSLClient(void);
    bool SSLconnect();
    bool recv(std::string &buf);
    bool send(std::string &buf);

private:
    bool verifyPeerCA(void);

private:
    bool verifyCA;
    std::string caFile;
    std::string certFile;
    std::string privateKeyFile;
};

}

#endif // SSLCLIENT_H
