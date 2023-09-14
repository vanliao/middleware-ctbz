#include "epollcommunicator.h"

#include <string.h>
#include <thread>
#include <sys/epoll.h>
#include <unistd.h>
#include "api.h"
#include "tinylog.h"

namespace network {

EpollCommunicator::EpollCommunicator(const std::string &serverIP, const int serverPort, const ServerType svrtype)
    :type(svrtype), port(serverPort), ip(serverIP), epollFd(-1), finish(false)
{
    return;
}

EpollCommunicator::~EpollCommunicator()
{
    if (-1 != epollFd)
    {
        close(epollFd);
    }

    return;
}

bool EpollCommunicator::start(EpollCommunicatorIF &obj)
{
    epollFd = epoll_create1(0);
    if (-1 == epollFd)
    {
        log_error("create epoll fd failed");
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

void EpollCommunicator::stop()
{
    finish = true;
    return;
}

bool EpollCommunicator::connect()
{
    std::shared_ptr<network::Socket> clt;
    if (TCP == type)
    {
        clt = std::make_shared<network::TcpClient>(ip, port);
        auto it = clients.insert(std::pair<unsigned int, std::shared_ptr<network::Socket> >
                                 (api::getClientID(), clt));
        if (it.second)
        {
            unsigned int connID = it.first->first;
            log_debug("create connection success:" << connID);
            network::TcpClient *sock = dynamic_cast<network::TcpClient *>(clt.get());
            bool ret = sock->connect();
            if (ret)
            {
                log_debug("connect succ");
                struct epoll_event epEvt;
                epEvt.data.fd = connID;
                epEvt.events = EPOLLOUT;/* 不用EPOLLIN是为了在epoll wait处理的时候通知connectNotify */
                int rc = epoll_ctl(epollFd, EPOLL_CTL_ADD, sock->fd, &epEvt);
                if (0 != rc)
                {
                    log_error("add event faild:" << strerror(errno));
                    clients.erase(it.first);
                    return false;
                }
            }
            else if (!ret && network::TcpClient::CONNECTING == sock->doing)
            {
                log_debug("connecting");
                struct epoll_event epEvt;
                epEvt.data.fd = connID;
                epEvt.events = EPOLLOUT;
                int rc = epoll_ctl(epollFd, EPOLL_CTL_ADD, sock->fd, &epEvt);
                if (0 != rc)
                {
                    log_error("add event faild:" << strerror(errno));
                    clients.erase(it.first);
                    return false;
                }
            }
            else
            {
                log_error("connect failed:" << strerror(errno));
                clients.erase(it.first);
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    else //UDP
    {
        clt = std::make_shared<network::UdpClient>(ip, port);
        auto it = clients.insert(std::pair<unsigned int, std::shared_ptr<network::Socket> >
                                 (api::getClientID(), clt));
        if (it.second)
        {
            unsigned int connID = it.first->first;
            log_debug("create connection success:" << connID);
            network::UdpClient *sock = dynamic_cast<network::UdpClient *>(clt.get());
            bool ret = sock->connect();
            if (!ret)
            {
                log_error("connect failed:" << strerror(errno));
                clients.erase(it.first);
                return false;
            }

            struct epoll_event epEvt = {0};
            epEvt.data.fd = connID;
            epEvt.events = EPOLLOUT;/* 不用EPOLLIN是为了在epoll wait处理的时候通知connectNotify */
            int rc = epoll_ctl(epollFd, EPOLL_CTL_ADD, sock->fd, &epEvt);
            if (0 != rc)
            {
                log_error("add event faild:" << strerror(errno));
                clients.erase(it.first);
                return false;
            }
        }
    }

    return true;
}

network::TcpClient *EpollCommunicator::getTcpClient(const int connID)
{
    auto it = clients.find(connID);
    if (clients.end() != it)
    {
        if (TCP == type)
        {
            return dynamic_cast<network::TcpClient*>(it->second.get());
        }
    }

    return NULL;
}

UdpClient *EpollCommunicator::getUdpClient(const int connID)
{
    auto it = clients.find(connID);
    if (clients.end() != it)
    {
        if (UDP == type)
        {
            return dynamic_cast<network::UdpClient*>(it->second.get());
        }
    }

    return NULL;
}

void EpollCommunicator::disconnect(const int connID)
{
    if (TCP == type)
    {
        network::TcpClient *clt = getTcpClient(connID);
        if (NULL != clt)
        {
            epoll_ctl(epollFd, EPOLL_CTL_DEL, clt->fd, NULL);
            auto it = clients.find(connID);
            if (clients.end() != it)
            {
                clients.erase(it);
                log_debug("remove connection:" << connID);
            }
        }
    }
    else
    {
        network::UdpClient *clt = getUdpClient(connID);
        if (NULL != clt)
        {
            epoll_ctl(epollFd, EPOLL_CTL_DEL, clt->fd, NULL);
            auto it = clients.find(connID);
            if (clients.end() != it)
            {
                clients.erase(it);
                log_debug("remove connection:" << connID);
            }
        }
    }

    return;
}

bool EpollCommunicator::startTcpSvr(EpollCommunicatorIF &obj)
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
                log_debug("tcp epoll in");
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
        }

    }

    return exitflag;
}

bool EpollCommunicator::startUdpSvr(EpollCommunicatorIF &obj)
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
                log_debug("udp epoll in");
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
        }

    }

    return exitflag;
}

}
