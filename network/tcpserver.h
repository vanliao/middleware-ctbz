#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <map>
#include "tcpsocket.h"
#include "tcpclient.h"

namespace network {

class TcpServer: public TcpSocket
{
public:
    TcpServer(const std::string servrIP, const int serverPort);
    virtual ~TcpServer(void);
    bool bind(void);
    bool listen(void);
    bool accept(unsigned int &connID);
    void close(const int connID);
    bool recv(std::string &/*buf*/);
    bool send(const std::string &/*buf*/);
    TcpClient *getClient(const unsigned int connID);
    TcpClient *getClient(void);
public:
    std::map<unsigned int, std::shared_ptr<TcpClient> > clients;
};

}
#endif // TCPSERVER_H
