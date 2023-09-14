#ifndef COMMONCOMMUNICATOR_H
#define COMMONCOMMUNICATOR_H

#include <thread>
#include <mutex>
#include <list>
#include "epollcommunicator.h"

namespace service {

class CommonCommunicatorIF;

class CommonCommunicator : public network::EpollCommunicator
{
public:
    CommonCommunicator(const std::string &serverIP, const int serverPort, const network::EpollCommunicator::ServerType type);
    virtual ~CommonCommunicator();
    bool open(void);
    bool start(CommonCommunicatorIF &obj);
    bool addExtenEvent(const int ev);
    bool delExtenEvent(const int ev);
    dev::EndPoint *getDev(const int connID);
    dev::EndPoint *getFirstDev(void);
    void getAllDev(std::vector<unsigned int> &vec);

private:
    bool startTcpSvr(CommonCommunicatorIF &obj);
    bool startUdpSvr(CommonCommunicatorIF &obj);

protected:
    std::vector<int> eventFds;
};

class CommonCommunicatorIF: public network::EpollCommunicatorIF
{
public:
    CommonCommunicatorIF(const std::string &serverIP, const int serverPort, const network::EpollCommunicator::ServerType svrtype):
        svr(serverIP, serverPort, svrtype){};
    virtual ~CommonCommunicatorIF(){};

    virtual void eventtNotify(const int event) = 0;

public:
    CommonCommunicator svr;
};

class Communicator : public CommonCommunicatorIF
{
public:
    Communicator(const std::string &serverIP, const int serverPort, const network::EpollCommunicator::ServerType type);
    ~Communicator();
    virtual void procHbTimer(void);
    bool start(void);
    void stop(void);
    void addEvent(std::shared_ptr<msg::Msg> &msg);
    void loop(void);
    void connectSvr(void);
    void disconnectSvr(const int connID);
    virtual void initExternEvent(void);

private:
    void run(void);

protected:
    int notifyEvt;
    int hbTimer;
    std::thread th;
    std::mutex mu;
    std::list<std::shared_ptr<msg::Msg> > msgList;
};
}

#endif // COMMONCOMMUNICATOR_H
