#ifndef COMMONCOMMUNICATOR_H
#define COMMONCOMMUNICATOR_H

#include <thread>
#include <mutex>
#include <list>
#include "epollcommunicator.h"

namespace service {

class CommonCommunicator : public network::EpollCommunicatorIF
{
public:
    CommonCommunicator(const std::string &serverIP, const int serverPort, const network::EpollCommunicator::ServerType type);
    virtual ~CommonCommunicator();
    bool start(void);
    void stop(void);
    void loop(void);
    void connectSvr(void);
    void addEvent(std::shared_ptr<msg::Msg> &msg);
    virtual void procHbTimer(void);

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
