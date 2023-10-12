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
    log_debug("epoll create fd " << epollFd);

    return true;
}

bool CommonCommunicator::send(const int connID, const std::string &buf)
{
    if (TCP == type || SSL == type)
    {
        network::TcpClient *clt = getTcpClient(connID);
        if (NULL == clt)
        {
            log_error("invalid ptr");
            return false;
        }
        clt->sendBuf.append(buf);

        struct epoll_event epEvt;
        epEvt.data.fd = connID;
        epEvt.events = EPOLLOUT|EPOLLIN;
        int rc = epoll_ctl(epollFd, EPOLL_CTL_MOD, clt->fd, &epEvt);
        if (0 != rc)
        {
            log_error("mod event faild:" << strerror(errno));
        }
    }
    else if (WS == type)
    {
        network::WebsocketClient *clt = dynamic_cast<network::WebsocketClient *>(getTcpClient(connID));
        if (NULL != clt)
        {
            clt->sendPrepare(buf, network::WebSocket::BINARY);
            struct epoll_event epEvt;
            epEvt.data.fd = connID;
            epEvt.events = EPOLLOUT|EPOLLIN;
            int rc = epoll_ctl(epollFd, EPOLL_CTL_MOD, clt->fd, &epEvt);
            if (0 != rc)
            {
                log_error("mod event faild:" << strerror(errno));
            }
        }
        else
        {
            log_error("invalid ptr");
            return false;
        }
    }
    else
    {
        network::UdpClient *clt = getUdpClient(connID);
        if (NULL == clt)
        {
            log_error("invalid ptr");
            return false;
        }
        clt->sendBuf.push_back(buf);

        struct epoll_event epEvt;
        epEvt.data.fd = connID;
        epEvt.events = EPOLLOUT|EPOLLIN;
        int rc = epoll_ctl(epollFd, EPOLL_CTL_MOD, clt->fd, &epEvt);
        if (0 != rc)
        {
            log_error("mod event faild:" << strerror(errno));
        }
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
    if (TCP == type || SSL == type)
    {
        ret = startTcpSvr(obj);
    }
    else if (WS == type)
    {
        ret = startWSSvr(obj);
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

dev::EndPoint *CommonCommunicator::getDev(const int connID)
{
    if (TCP == type || WS == type || SSL == type)
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

dev::EndPoint *CommonCommunicator::getFirstDev()
{
    auto it = clients.begin();
    if (clients.end() != it)
    {
        if (TCP == type || WS == type || SSL == type)
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

bool CommonCommunicator::startTcpSvr(CommonCommunicatorIF &obj)
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
                log_debug("epoll error " << connID);
                obj.closeNotify(connID);
                disconnect(connID);
                continue;
            }
            if (waitEv[i].events & EPOLLOUT)
            {
                log_debug("epoll out " << connID);
                network::TcpClient *clt = getTcpClient(connID);
                if (NULL != clt)
                {
                    if (network::TcpClient::CONNECTING == clt->status)
                    {
                        if (SSL == type)
                        {
                            network::SSLClient *sslClt = dynamic_cast<network::SSLClient *>(clt);
                            if (!sslClt->SSLconnect())
                            {
                                log_error("ssl connect failed");
                                disconnect(connID);
                            }
                        }
                        else
                        {
                            struct epoll_event epEvt;
                            epEvt.data.fd = connID;
                            epEvt.events = EPOLLIN;
                            epoll_ctl(epollFd, EPOLL_CTL_MOD, clt->fd, &epEvt);
                            clt->status = network::TcpClient::CONNECTED;
                            clt->connID = connID;
                            obj.connectNotify(connID);
                        }
                    }
                    else if (network::TcpClient::SSLCONNECTING == clt->status)
                    {
                        struct epoll_event epEvt;
                        epEvt.data.fd = connID;
                        epEvt.events = EPOLLIN;
                        epoll_ctl(epollFd, EPOLL_CTL_MOD, clt->fd, &epEvt);
                        clt->status = network::TcpClient::CONNECTED;
                        clt->connID = connID;
                        obj.connectNotify(connID);
                    }
                    else
                    {
                        if (clt->send(clt->sendBuf))
                        {
                            struct epoll_event epEvt;
                            epEvt.data.fd = connID;
                            epEvt.events = EPOLLIN;
                            epoll_ctl(epollFd, EPOLL_CTL_MOD, clt->fd, &epEvt);
                        }
                    }
                }
            }
            if (waitEv[i].events & EPOLLIN)
            {
                log_debug("epoll in " << connID);
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
                        if (clt->recv(buf))
                        {
                            if (0 != buf.size())
                            {
                                obj.recvNotify(connID, buf);
                            }
                            //else{}
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

bool CommonCommunicator::startWSSvr(CommonCommunicatorIF &obj)
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
                log_debug("epoll error " << connID);
                obj.closeNotify(connID);
                network::WebsocketClient *clt = dynamic_cast<network::WebsocketClient *>(getTcpClient(connID));
                clt->closeStatus = network::WebSocket::CLOSE_FORCE;
                disconnect(connID);
                continue;
            }
            if (waitEv[i].events & EPOLLOUT)
            {
                log_debug("epoll out " << connID);
                network::WebsocketClient *clt = dynamic_cast<network::WebsocketClient *>(getTcpClient(connID));
                if (NULL != clt)
                {
                    if (network::TcpClient::CONNECTING == clt->status)
                    {
                        struct epoll_event epEvt;
                        epEvt.data.fd = connID;
                        epEvt.events = EPOLLIN;
                        epoll_ctl(epollFd, EPOLL_CTL_MOD, clt->fd, &epEvt);
                        clt->status = network::TcpClient::CONNECTED;
                        clt->connID = connID;
                        clt->connectWS();
                        obj.connectNotify(connID);
                    }
                    else
                    {
                        network::WebsocketClient *clt = dynamic_cast<network::WebsocketClient *>(getTcpClient(connID));
                        if (NULL != clt)
                        {
                            while (!clt->sendBuf.empty())
                            {
                                std::string &s = clt->sendBuf.front();
                                if (clt->send(s))
                                {
                                    clt->sendBuf.pop_front();
                                    if (clt->sendBuf.empty())
                                    {
                                        if (network::WebSocket::CLOSE_COMPLETE == clt->closeStatus)
                                        {
                                            log_debug("epoll out send ws close msg and close ws");
                                            obj.closeNotify(connID);
                                            epoll_ctl(epollFd, EPOLL_CTL_DEL, clt->fd, NULL);
                                            disconnect(connID);
                                        }
                                        else
                                        {
                                            struct epoll_event epEvt;
                                            epEvt.data.fd = connID;
                                            epEvt.events = EPOLLIN;
                                            epoll_ctl(epollFd, EPOLL_CTL_MOD, clt->fd, &epEvt);
                                        }
                                        break;
                                    }
                                    //else {} 继续发送
                                }
                                else
                                {
                                    /* 发送失败,下次发送 */
                                    break;
                                }
                            }// while (!clt->sendBuf.empty())
                        }
                    }
                }
            }
            if (waitEv[i].events & EPOLLIN)
            {
                log_debug("epoll in " << connID);
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
                    network::WebsocketClient *clt = dynamic_cast<network::WebsocketClient *>(getTcpClient(connID));
                    if (NULL != clt)
                    {
                        std::string buf = "";
                        if (clt->recv(buf))
                        {
                            if (!buf.empty())
                            {
                                if (network::WebSocket::CLOSE_RECV == clt->closeStatus)
                                {
                                    log_debug("recv ws close and will send close rsp");
                                    /* 发送 ws close 消息 */
                                    sendWS(connID, buf, network::WebSocket::CLOSE);
                                    clt->closeSend();
                                }
                                else if (network::WebSocket::PONG == clt->wsOpCode)
                                {
                                    /* 收到 ws pong 消息 不需要处理 */
                                    log_debug("recv ws pong");
                                }
                                else if (network::WebSocket::PING == clt->wsOpCode)
                                {
                                    /* 发送 ws pong 消息 */
                                    log_debug("recv ws ping and will send pong");
                                    sendWS(connID, buf, network::WebSocket::PONG);
                                }
                                else
                                {
                                    obj.recvNotify(connID, buf);
                                }
                            }
                            else
                            {
                                if (network::WebSocket::PONG == clt->wsOpCode)
                                {
                                    /* 收到 ws pong 消息 不需要处理 */
                                    log_debug("recv ws pong");
                                }
                                else if(network::WebSocket::CLOSE_FORCE    == clt->closeStatus ||
                                        network::WebSocket::CLOSE_COMPLETE == clt->closeStatus)
                                {
                                    log_debug("epoll in recv empty or recv close msg should close");
                                    /* 收到 close 或者对端强制关闭 TCP */
                                    obj.closeNotify(connID);
                                    disconnect(connID);
                                }
                                else
                                {
                                    log_error("epoll in recv empty ws close status " << clt->closeStatus);
                                }
                            }
                        }
                        else
                        {
                            /* 握手失败 */
                            if(network::WebSocket::CLOSE_FORCE    == clt->closeStatus ||
                               network::WebSocket::CLOSE_COMPLETE == clt->closeStatus)
                            {
                                log_debug("epoll in recv fail should close ws");
                                obj.closeNotify(connID);
                                disconnect(connID);
                            }
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

bool CommonCommunicator::startUdpSvr(CommonCommunicatorIF &obj)
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
                log_debug("epoll error " << connID);
                obj.closeNotify(connID);
                disconnect(connID);
                continue;
            }
            if (waitEv[i].events & EPOLLOUT)
            {
                log_debug("epoll out " << connID);
                network::UdpClient *clt = getUdpClient(connID);
                if (NULL != clt)
                {
                    if (network::UdpClient::CONNECTING == clt->status)
                    {
                        struct epoll_event epEvt;
                        epEvt.data.fd = connID;
                        epEvt.events = EPOLLIN;
                        epoll_ctl(epollFd, EPOLL_CTL_MOD, clt->fd, &epEvt);
                        clt->status = network::UdpClient::CONNECTED;
                        clt->connID = connID;
                        obj.connectNotify(connID);
                    }
                    else
                    {
                        while (!clt->sendBuf.empty())
                        {
                            std::string &s = clt->sendBuf.front();
                            if (clt->send(s))
                            {
                                clt->sendBuf.pop_front();
                                if (clt->sendBuf.empty())
                                {
                                    struct epoll_event epEvt;
                                    epEvt.data.fd = connID;
                                    epEvt.events = EPOLLIN;
                                    epoll_ctl(epollFd, EPOLL_CTL_MOD, clt->fd, &epEvt);
                                    break;
                                }
                                //else {} 继续发送
                            }
                            else
                            {
                                /* 发送失败,下次发送 */
                                break;
                            }
                        }// while (!clt->sendBuf.empty())
                    }
                }
            }
            if (waitEv[i].events & EPOLLIN)
            {
                log_debug("epoll in " << connID);
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

Communicator::~Communicator()
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

void Communicator::procHbTimer()
{
    uint64_t buf;
    read(hbTimer, &buf, sizeof(uint64_t));
    log_debug("heartbeat timerNotify:" << hbTimer << " " << buf);

    dev::EndPoint *ep = model.getFirstDev();
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

void Communicator::stop()
{
    return model.stop();
}

void Communicator::addEvent(std::shared_ptr<msg::Msg> &msg)
{
    {
    std::lock_guard<std::mutex> locker(mu);
    msgList.push_back(msg);
    }
    uint64_t buf = 1;
    write(notifyEvt, &buf, sizeof(uint64_t));

}

void Communicator::loop()
{
    if (th.joinable())
    {
        th.join();
    }

    return;
}

void Communicator::connectSvr()
{
    bool ret = model.connect();
    if (!ret)
    {
        log_error("connect server failed");
        return;
    }
    return;
}

void Communicator::disconnectSvr(const int connID)
{
    model.disconnect(connID);
}

void Communicator::initExternEvent()
{
    model.addExtenEvent(notifyEvt);
    model.addExtenEvent(hbTimer);
    return;
}

void Communicator::run()
{
    notifyEvt = eventfd(0, 0);
    if (-1 == notifyEvt)
    {
        log_error("create event failed:" << strerror(errno));
        return;
    }
    log_debug("event create fd " << notifyEvt);

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
    log_debug("timer create fd " << hbTimer);

    bool ret = model.open();
    if (ret)
    {
        initExternEvent();
        model.start(*this);
    }

    return;
}

}
