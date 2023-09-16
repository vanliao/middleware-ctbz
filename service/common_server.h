#ifndef COMMONSERVER_H
#define COMMONSERVER_H

#include <vector>
#include <list>
#include <thread>
#include <mutex>
#include "epollserver.h"

namespace service {

class CommonServerIF;

class CommonServer  : public network::EpollServer
{
public:
    CommonServer(const std::string &serverIP, const int serverPort, const ServerType type);
    virtual ~CommonServer();
    bool open(void);
    bool send(const int connID, const std::string &buf);
    void closeDev(const unsigned int connID);
    bool start(CommonServerIF &obj);
    bool addExtenEvent(const int ev);
    bool delExtenEvent(const int ev);
    dev::EndPoint *getDev(const int connID);
    dev::EndPoint *getFirstDev(void);
    void getAllDev(std::vector<unsigned int> &vec);

private:
    bool startTcpSvr(CommonServerIF &obj);
    bool startUdpSvr(CommonServerIF &obj);

private:
    std::vector<int> eventFds;
};

class CommonServerIF: public network::EpollServerIF
{
public:
    CommonServerIF(const std::string serverIP, const int serverPort, const network::EpollServer::ServerType type):
        model(serverIP, serverPort, type){};
    virtual ~CommonServerIF(){};

    virtual void eventtNotify(const int event) = 0;

protected:
    CommonServer model; //框架模型
};

class Server: public CommonServerIF
{
public:
    Server(const std::string serverIP, const int serverPort, const network::EpollServer::ServerType type);
    virtual ~Server();
    bool start(void);
    void stop(void);
    void addEvent(std::shared_ptr<msg::Msg> &msg);
    void loop(void);
    virtual void initExternEvent(void);

private:
    void run(void);

protected:
    std::thread th;
    int notifyEvt;
    std::mutex mu;
    std::list<std::shared_ptr<msg::Msg> > msgList;
};

}

#endif // COMMONSERVER_H
