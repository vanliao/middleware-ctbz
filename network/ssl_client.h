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
    POLL_RESULT pollIn(void);
    POLL_RESULT pollOut(void);

    bool verifyPeerCA(void);
    POLL_RESULT connectTCPAction(void);
    POLL_RESULT connectSSLAction(void);
    POLL_RESULT sendAction(void);

protected:
    bool verifyCA;
    std::string caFile;
    std::string certFile;
    std::string privateKeyFile;
    std::function<POLL_RESULT(void)> pollInAction;
    std::function<POLL_RESULT(void)> pollOutAction;
};

}

#endif // SSLCLIENT_H
