#include <sys/epoll.h>
#include <string.h>
#include <functional>
#include <unistd.h>
#include <sys/eventfd.h>
#include "common_server.h"
#include "udpserver.h"
#include "tinylog.h"

namespace service {

CommonServer::CommonServer(const std::string &serverIP, const int serverPort, const ServerType type) :
    network::EpollServer(serverIP, serverPort, type)
{
    return;
}

CommonServer::~CommonServer()
{
    return;
}

bool CommonServer::open()
{
    epollFd = epoll_create1(0);
    if (-1 == epollFd)
    {
        log_error("create epoll fd failed");
        return false;
    }

    return true;
}

void CommonServer::closeDev(const unsigned int connID)
{
    if (TCP == svrType)
    {
        network::TcpServer *svr = dynamic_cast<network::TcpServer*>(sock.get());
        if (NULL != svr)
        {
            network::TcpClient *clt = svr->getClient(connID);
            if (NULL != clt)
            {
                epoll_ctl(epollFd, EPOLL_CTL_DEL, clt->fd, NULL);
                svr->close(connID);
            }
        }
        else
        {
            log_error("get tcp server failed");
        }
    }
    else
    {
        network::UdpServer *svr = dynamic_cast<network::UdpServer*>(sock.get());
        if (NULL != svr)
        {
            network::UdpClient *clt = svr->getClient(connID);
            if (NULL != clt)
            {
                svr->close(connID);
            }
        }
        else
        {
            log_error("get udp server failed");
        }
    }

    return;
}

bool CommonServer::start(CommonServerIF &obj)
{
    if (-1 == epollFd)
    {
        log_error("invalid epolll fd");
        return false;
    }

    bool ret = true;
    if (TCP == svrType)
    {
        ret = startTcpSvr(obj);
    }
    else
    {
        ret = startUdpSvr(obj);
    }

    return ret;
}

bool CommonServer::addExtenEvent(const int ev)
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

bool CommonServer::delExtenEvent(const int ev)
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
            log_error("delete event faild:" << strerror(errno));
            return false;
        }
        break;
    }

    return true;
}

dev::EndPoint *CommonServer::getDev(const int connID)
{
    if (TCP == svrType)
    {
        network::TcpServer *svr = dynamic_cast<network::TcpServer*>(sock.get());
        network::TcpClient *clt = svr->getClient(connID);
        return dynamic_cast<dev::EndPoint *>(clt);
    }
    else
    {
        network::UdpServer *svr = dynamic_cast<network::UdpServer*>(sock.get());
        network::UdpClient *clt = svr->getClient(connID);
        return dynamic_cast<dev::EndPoint *>(clt);
    }

    return NULL;
}

dev::EndPoint *CommonServer::getFirstDev()
{
    if (TCP == svrType)
    {
        network::TcpServer *svr = dynamic_cast<network::TcpServer*>(sock.get());
        network::TcpClient *clt = svr->getClient();
        return dynamic_cast<dev::EndPoint *>(clt);
    }
    else
    {
        network::UdpServer *svr = dynamic_cast<network::UdpServer*>(sock.get());
        network::UdpClient *clt = svr->getClient();
        return dynamic_cast<dev::EndPoint *>(clt);
    }

    return NULL;
}

void CommonServer::getAllDev(std::vector<unsigned int> &vec)
{
    if (TCP == svrType)
    {
        network::TcpServer *svr = dynamic_cast<network::TcpServer*>(sock.get());
        for (const auto &it : svr->clients)
        {
            vec.push_back(it.first);
        }
    }
    else
    {
        network::UdpServer *svr = dynamic_cast<network::UdpServer*>(sock.get());
        for (const auto &it : svr->clients)
        {
            vec.push_back(it.first);
        }
    }

    return;
}

bool CommonServer::startTcpSvr(CommonServerIF &obj)
{
    network::TcpServer *svr = dynamic_cast<network::TcpServer*>(sock.get());
    if (NULL == svr)
    {
        log_error("create tcp server failed");
        return false;
    }

    bool ret = true;
    ret &= svr->create();
    ret &= svr->bind();
    ret &= svr->listen();
    if (!ret)
    {
        log_error("create tcp server failed");
        return false;
    }

    struct epoll_event epEvt;
    epEvt.data.fd = svr->fd;
    epEvt.events = EPOLLIN;

    int rc = epoll_ctl(epollFd, EPOLL_CTL_ADD, svr->fd, &epEvt);
    if (0 != rc)
    {
        log_error("add server fd to epoll failed:" << strerror(errno));
        return false;
    }

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
            if ((waitEv[i].events & EPOLLERR) ||
                (waitEv[i].events & EPOLLHUP))
            {
                log_debug("epoll err");
                if (svr->fd == waitEv[i].data.fd)
                {
                    log_error("tcp server error:" << strerror(errno));
                    exitEpoll();
                    break;
                }
                else
                {
                    unsigned int connID = waitEv[i].data.fd;
                    network::TcpClient *clt = svr->getClient(connID);
                    if (NULL != clt)
                    {
                        obj.closeNotify(connID);
                        epoll_ctl(epollFd, EPOLL_CTL_DEL, clt->fd, NULL);
                        svr->close(connID);
                    }
                }
            }
            else if (svr->fd == waitEv[i].data.fd)
            {
                log_debug("epoll accept");
                unsigned int connID = 0;
                ret = svr->accept(connID);
                if (ret)
                {
                    epEvt.data.fd = connID;
                    epEvt.events = EPOLLIN;
                    int rc = epoll_ctl(epollFd, EPOLL_CTL_ADD, svr->getClient(connID)->fd, &epEvt);
                    if (0 != rc)
                    {
                        log_error("add tcp client fd to epoll failed:" << strerror(errno));
                    }
                    else
                    {
                        obj.connectNotify(connID);
                    }
                }
            }
            else
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
                    unsigned int connID = waitEv[i].data.fd;
                    network::TcpClient *clt = svr->getClient(connID);
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
                            epoll_ctl(epollFd, EPOLL_CTL_DEL, clt->fd, NULL);
                            svr->close(connID);
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

bool CommonServer::startUdpSvr(CommonServerIF &obj)
{
    network::UdpServer *svr = dynamic_cast<network::UdpServer*>(sock.get());
    if (NULL == svr)
    {
        log_error("create udp server failed");
        return false;
    }

    bool ret = true;
    ret &= svr->create();
    ret &= svr->bind();
    if (!ret)
    {
        log_error("create udp server failed");
        return false;
    }

    struct epoll_event epEvt;
    epEvt.data.fd = svr->fd;
    epEvt.events = EPOLLIN;

    int rc = epoll_ctl(epollFd, EPOLL_CTL_ADD, svr->fd, &epEvt);
    if (0 != rc)
    {
        log_error("add server fd to epoll failed:" << strerror(errno));
        return false;
    }

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
            if ((waitEv[i].events & EPOLLERR) ||
                (waitEv[i].events & EPOLLHUP))
            {
                log_debug("epoll err");
                if (svr->fd == waitEv[i].data.fd)
                {
                    log_error("udp server error:" << strerror(errno));
                    exitEpoll();
                    break;
                }
            }
            else if (svr->fd == waitEv[i].data.fd)
            {
                log_debug("epoll in");
                std::string buf;
                unsigned int connID = 0;
                if (svr->accept(buf, connID))
                {
                    obj.connectNotify(connID);
                }

                if (0 != buf.size())
                {
                    obj.recvNotify(connID, buf);
                }
                else
                {
                    log_warning("udp svr recv zero msg");
                }

            }
            else
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

                if (it != eventFds.end())
                {
                    obj.eventtNotify(*it);
                }
            }
        }
    }

    return exitflag;
}

Server::Server(const std::string serverIP, const int serverPort, const network::EpollServer::ServerType type):
    CommonServerIF(serverIP, serverPort, type)
{
    return;
}

Server::~Server()
{
    if (-1 != notifyEvt)
    {
        close(notifyEvt);
        notifyEvt = -1;
    }
    return;
}

bool Server::start()
{
    th = std::thread(std::bind(&Server::run, this));
    return true;
}

void Server::stop()
{
    return svr.stop();
}

void Server::addEvent(std::shared_ptr<msg::Msg> &msg)
{
    {
    std::lock_guard<std::mutex> locker(mu);
    msgList.push_back(msg);
    }
    uint64_t buf = 1;
    write(notifyEvt, &buf, sizeof(uint64_t));

    return;
}

void Server::loop()
{
    if (th.joinable())
    {
        th.join();
    }

    return;
}

void Server::run()
{
    notifyEvt = eventfd(0, 0);
    if (-1 == notifyEvt)
    {
        log_error("create event failed:" << strerror(errno));
        return;
    }

    bool ret = svr.open();
    if (ret)
    {
        svr.addExtenEvent(notifyEvt);
        svr.start(*this);
    }

    return;
}

}
