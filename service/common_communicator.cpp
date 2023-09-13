#include <unistd.h>
#include <functional>
#include <string.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include "common_communicator.h"
#include "tinylog.h"

namespace service {

CommonCommunicator::CommonCommunicator(const std::string &serverIP, const int serverPort, const network::EpollCommunicator::ServerType type):
        network::EpollCommunicatorIF(serverIP, serverPort, type)
{
    notifyEvt = -1;
    hbTimer = -1;

    return;
}

CommonCommunicator::~CommonCommunicator()
{
    if (-1 != notifyEvt)
    {
        close(notifyEvt);
    }

    if (-1 != hbTimer)
    {
        close(hbTimer);
    }

    return;
}

bool CommonCommunicator::start()
{
    th = std::thread(std::bind(&CommonCommunicator::run, this));
    return true;
}

void CommonCommunicator::stop()
{
    svr.stop();
}

void CommonCommunicator::loop()
{
    if (th.joinable())
    {
        th.join();
    }

    return;
}

void CommonCommunicator::connectSvr()
{
    bool ret = svr.connect();
    if (!ret)
    {
        log_error("connect server failed");
        return;
    }
    return;
}

void CommonCommunicator::addEvent(std::shared_ptr<msg::Msg> &msg)
{
    std::lock_guard<std::mutex> locker(mu);
    msgList.push_back(msg);
    uint64_t buf = 1;
    write(notifyEvt, &buf, sizeof(uint64_t));
    return;
}

void CommonCommunicator::run()
{
    notifyEvt = eventfd(0, 0);
    if (-1 == notifyEvt)
    {
        log_error("create event failed:" << strerror(errno));
        return;
    }

    struct itimerspec new_value;
    new_value.it_value.tv_sec = 10;
    new_value.it_value.tv_nsec = 0;
    new_value.it_interval.tv_sec = 10;
    new_value.it_interval.tv_nsec = 0;
    hbTimer = timerfd_create(CLOCK_MONOTONIC, 0);
    if (-1 == hbTimer)
    {
        log_error("create hb timer failed:" << strerror(errno));
        return;
    }
    int rc = timerfd_settime(hbTimer, 0, &new_value, NULL);
    if (rc < 0)
    {
        log_error("set hb timer failed:" << strerror(errno));
        close(hbTimer);
        close(notifyEvt);
        hbTimer = -1;
        notifyEvt = -1;
        return;
    }

    bool ret = svr.open();
    if (ret)
    {
        svr.addExtenEvent(notifyEvt);
        svr.addExtenEvent(hbTimer);
        svr.start(*this);
    }

    return;
}

void CommonCommunicator::procHbTimer()
{
    uint64_t buf;
    read(hbTimer, &buf, sizeof(uint64_t));
    log_debug("heartbeat timerNotify:" << hbTimer << " " << buf);

    dev::EndPoint *ep = svr.getFirstDev();
    if (NULL == ep)
    {
        log_warning("reconnect server");
        connectSvr();
    }
//    else
//    {
//        SendHeartBeat();
//    }
}

}
