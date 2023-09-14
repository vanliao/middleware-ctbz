#include <unistd.h>
#include <functional>
#include <string.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include "common_communicator.h"
#include "tinylog.h"

namespace service {

CommonCommunicator::CommonCommunicator(const std::string &serverIP, const int serverPort, const network::EpollCommunicator::ServerType type):
        network::EpollCommunicator(serverIP, serverPort, type)
{
    return;
}

CommonCommunicator::~CommonCommunicator()
{
}

bool CommonCommunicator::open()
{
    epollFd = epoll_create1(0);
    if (-1 == epollFd)
    {
        log_error("create epoll fd failed");
        return false;
    }

    return true;
}

bool CommonCommunicator::start(CommonCommunicatorIF &obj)
{
    if (-1 == epollFd)
    {
        log_error("invalid epolll fd");
        return false;
    }

    bool ret = true;
    if (TCP == type)
    {
        ret = startTcpSvr(obj);
    }
    else
    {
        ret = startUdpSvr(obj);
    }

    return ret;
}

bool CommonCommunicator::addExtenEvent(const int ev)
{
    eventFds.push_back(ev);

    struct epoll_event epEvt;
    epEvt.data.fd = ev;
    epEvt.events = EPOLLIN;
    int rc = epoll_ctl(epollFd, EPOLL_CTL_ADD, ev, &epEvt);
    if (0 != rc)
    {
        log_error("add event faild:" << strerror(errno));
        return false;
    }

    return true;
}

bool CommonCommunicator::delExtenEvent(const int ev)
{
    for(auto it = eventFds.begin(); eventFds.end() != it; it++)
    {
        if (*it != ev)
        {
            continue;
        }
        eventFds.erase(it);

        int rc = epoll_ctl(epollFd, EPOLL_CTL_DEL, ev, NULL);
        if (0 != rc)
        {
            log_error("add event faild:" << strerror(errno));
            return false;
        }
        break;
    }

    return true;
}

dev::EndPoint *service::CommonCommunicator::getDev(const int connID)
{
    if (TCP == type)
    {
        network::TcpClient *clt = getTcpClient(connID);
        return dynamic_cast<dev::EndPoint *>(clt);
    }
    else //UDP
    {
        network::UdpClient *clt = getUdpClient(connID);
        return dynamic_cast<dev::EndPoint *>(clt);
    }

    return NULL;
}

dev::EndPoint *service::CommonCommunicator::getFirstDev()
{
    auto it = clients.begin();
    if (clients.end() != it)
    {
        if (TCP == type)
        {
            network::TcpClient *clt = dynamic_cast<network::TcpClient*>(it->second.get());
            return dynamic_cast<dev::EndPoint *>(clt);
        }
        else
        {
            network::UdpClient *clt = dynamic_cast<network::UdpClient*>(it->second.get());
            return dynamic_cast<dev::EndPoint *>(clt);
        }
    }

    return NULL;
}

void CommonCommunicator::getAllDev(std::vector<unsigned int> &vec)
{
    for (const auto &it : clients)
    {
        vec.push_back(it.first);
    }
    return;

}

bool service::CommonCommunicator::startTcpSvr(CommonCommunicatorIF &obj)
{
    bool exitflag = true;
    auto exitEpoll = [this, &exitflag]()
    {
        finish = true;
        exitflag = false;
    };

    struct epoll_event waitEv[10];
    memset(waitEv, 0, 10*sizeof(struct epoll_event));
    while (!finish)
    {
        int num = epoll_wait(epollFd, waitEv, 10, 1000);
        if (-1 == num)
        {
#ifndef MIPS
            if (EINTR != errno)
            {
                log_error("epoll wait failed:" << strerror(errno));
                exitEpoll();
            }
#else
            ef_log_error("epoll wait failed on MIPS")
#endif
        }

        for (int i = 0; i < num; i++)
        {
            unsigned int connID = waitEv[i].data.fd;
            if ((waitEv[i].events & EPOLLERR) ||
                (waitEv[i].events & EPOLLHUP))
            {
                log_debug("epoll error");
                obj.closeNotify(connID);
                disconnect(connID);
            }
            else if (waitEv[i].events & EPOLLOUT)
            {
                log_debug("epoll out");
                network::TcpClient *clt = getTcpClient(connID);
                if (NULL != clt)
                {
                    struct epoll_event epEvt;
                    epEvt.data.fd = connID;
                    epEvt.events = EPOLLIN;
                    epoll_ctl(epollFd, EPOLL_CTL_MOD, clt->fd, &epEvt);
                    obj.connectNotify(connID);
                }
            }
            else if (waitEv[i].events & EPOLLIN)
            {
                log_debug("epoll in");
                auto it = eventFds.begin();
                while(eventFds.end() != it)
                {
                    if (*it == waitEv[i].data.fd)
                    {
                        break;
                    }
                    ++it;
                }

                if (it == eventFds.end())
                {
                    network::TcpClient *clt = getTcpClient(connID);
                    if (NULL != clt)
                    {
                        std::string buf;
                        clt->recv(buf);
                        if (0 != buf.size())
                        {
                            obj.recvNotify(connID, buf);
                        }
                        else
                        {
                            obj.closeNotify(connID);
                            disconnect(connID);
                        }
                    }
                }
                else
                {
                    obj.eventtNotify(*it);
                }
            }
        }

    }

    return exitflag;
}

bool service::CommonCommunicator::startUdpSvr(CommonCommunicatorIF &obj)
{
    bool exitflag = true;
    auto exitEpoll = [this, &exitflag]()
    {
        finish = true;
        exitflag = false;
    };

    struct epoll_event waitEv[10];
    memset(waitEv, 0, 10*sizeof(struct epoll_event));
    while (!finish)
    {
        int num = epoll_wait(epollFd, waitEv, 10, 1000);
        if (-1 == num)
        {
#ifndef MIPS
            if (EINTR != errno)
            {
                log_error("epoll wait failed:" << strerror(errno));
                exitEpoll();
            }
#else
            ef_log_error("epoll wait failed on MIPS")
#endif
        }

        for (int i = 0; i < num; i++)
        {
            unsigned int connID = waitEv[i].data.fd;
            if ((waitEv[i].events & EPOLLERR) ||
                (waitEv[i].events & EPOLLHUP))
            {
                log_debug("epoll error");
                obj.closeNotify(connID);
                disconnect(connID);
            }
            else if (waitEv[i].events & EPOLLOUT)
            {
                log_debug("epoll out");
                network::UdpClient *clt = getUdpClient(connID);
                if (NULL != clt)
                {
                    struct epoll_event epEvt;
                    epEvt.data.fd = connID;
                    epEvt.events = EPOLLIN;
                    epoll_ctl(epollFd, EPOLL_CTL_MOD, clt->fd, &epEvt);
                    obj.connectNotify(connID);
                }
            }
            else if (waitEv[i].events & EPOLLIN)
            {
                log_debug("epoll in");
                auto it = eventFds.begin();
                while(eventFds.end() != it)
                {
                    if (*it == waitEv[i].data.fd)
                    {
                        break;
                    }
                    ++it;
                }

                if (it == eventFds.end())
                {
                    network::UdpClient *clt = getUdpClient(connID);
                    if (NULL != clt)
                    {
                        std::string buf;
                        clt->recv(buf);
                        if (0 != buf.size())
                        {
                            obj.recvNotify(connID, buf);
                        }
                        else
                        {
                            obj.closeNotify(connID);
                            disconnect(connID);
                        }
                    }
                }
                else
                {
                    obj.eventtNotify(*it);
                }
            }
        }

    }

    return exitflag;
}

Communicator::Communicator(const std::string &serverIP, const int serverPort, const network::EpollCommunicator::ServerType type) :
    CommonCommunicatorIF(serverIP, serverPort, type)
{
    notifyEvt = -1;
    hbTimer = -1;
}

service::Communicator::~Communicator()
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

void service::Communicator::procHbTimer()
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

bool Communicator::start()
{
    th = std::thread(std::bind(&Communicator::run, this));
    return true;
}

void service::Communicator::stop()
{
    return svr.stop();
}

void service::Communicator::addEvent(std::shared_ptr<msg::Msg> &msg)
{
    {
    std::lock_guard<std::mutex> locker(mu);
    msgList.push_back(msg);
    }
    uint64_t buf = 1;
    write(notifyEvt, &buf, sizeof(uint64_t));

}

void service::Communicator::loop()
{
    if (th.joinable())
    {
        th.join();
    }

    return;
}

void Communicator::connectSvr()
{
    bool ret = svr.connect();
    if (!ret)
    {
        log_error("connect server failed");
        return;
    }
    return;
}

void service::Communicator::disconnectSvr(const int connID)
{
    svr.disconnect(connID);
}

void service::Communicator::initExternEvent()
{
    svr.addExtenEvent(notifyEvt);
    svr.addExtenEvent(hbTimer);
    return;
}

void service::Communicator::run()
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
        initExternEvent();
        svr.start(*this);
    }

    return;
}

}
