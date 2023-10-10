#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "epoll_server.h"
#include "tinylog.h"

namespace network {

EpollServer::EpollServer(const std::string &serverIP, const int serverPort, const ServerType type)
{
    epollFd = -1;
    svrType = type;
    finish = false;
    if (TCP == type)
    {
        sock = std::make_shared<TcpServer>(serverIP, serverPort);
//        TcpServer *cvt = dynamic_cast<TcpServer*>(sock.get());
    }
    else if (SSL == type)
    {
        sock = std::make_shared<SSLServer>(serverIP, serverPort);
    }
    else if (WS == type)
    {
        sock = std::make_shared<WebsocketServer>(serverIP, serverPort);
    }
    else
    {
        sock = std::make_shared<UdpServer>(serverIP, serverPort);
    }
    return;
}

EpollServer::~EpollServer()
{
    if (-1 != epollFd)
    {
        close(epollFd);
        epollFd = -1;
    }
    return;
}

bool EpollServer::start(EpollServerIF &obj)
{
    epollFd = epoll_create1(0);
    if (-1 == epollFd)
    {
        log_error("create epoll fd failed");
        return false;
    }

    bool ret = true;
    if (TCP == svrType || SSL == svrType)
    {
        ret = startTcpSvr(obj);
    }
    else if (WS == svrType)
    {
        ret = startWSSvr(obj);
    }
    else
    {
        ret = startUdpSvr(obj);
    }

    return ret;
}

void EpollServer::stop()
{
    finish = true;
}

void EpollServer::setSSLPemFile(const std::string &certFile, const std::string &keyFile)
{
    if (SSL == svrType)
    {
        SSLServer *svr = dynamic_cast<SSLServer *>(sock.get());
        svr->setCertificateFile(certFile);
        svr->setPrivateKeyFile(keyFile);
    }
    else
    {
        log_error("set pem file only for ssl");
    }

    return;
}

bool EpollServer::startTcpSvr(EpollServerIF &obj)
{
    TcpServer *svr = dynamic_cast<TcpServer*>(sock.get());
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
#ifndef MIPS
        if (-1 == num)
        {
            if (EINTR != errno)
            {
                log_error("epoll wait failed:" << strerror(errno));
                exitEpoll();
            }
        }
#else
            ef_log_error("epoll wait failed on MIPS")
#endif

        for (int i = 0; i < num; i++)
        {
            if ((waitEv[i].events & EPOLLERR) ||
                (waitEv[i].events & EPOLLHUP))
            {
                if (svr->fd == waitEv[i].data.fd)
                {
                    log_error("tcp server error:" << strerror(errno));
                    exitEpoll();
                    break;
                }
                else
                {
                    unsigned int connID = waitEv[i].data.fd;
                    obj.closeNotify(connID);
                    epoll_ctl(epollFd, EPOLL_CTL_DEL, connID, NULL);
                    svr->close(connID);
                }
            }
            else if (svr->fd == waitEv[i].data.fd)
            {
                unsigned int connID = 0;
                ret = svr->accept(connID);
                if (ret)
                {
                    epEvt.data.fd = connID;
                    epEvt.events = EPOLLIN;
                    rc = epoll_ctl(epollFd, EPOLL_CTL_ADD, connID, &epEvt);
                    if (0 != rc)
                    {
                        log_error("add server fd to epoll failed:" << strerror(errno));
                        exitEpoll();
                        break;
                    }
                    obj.connectNotify(connID);
                }
            }
            else
            {
                unsigned int connID = waitEv[i].data.fd;
                TcpClient *clt = svr->getClient(connID);
                if (NULL != clt)
                {
                    log_debug("tcp epoll in");
                    std::string buf;
                    clt->recv(buf);
                    obj.recvNotify(connID, buf);
                }
            }
        }
    }

    return exitflag;
}

bool EpollServer::startWSSvr(EpollServerIF &obj)
{
    WebsocketServer *svr = dynamic_cast<WebsocketServer*>(sock.get());
    if (NULL == svr)
    {
        log_error("create ws server failed");
        return false;
    }

    bool ret = true;
    ret &= svr->create();
    ret &= svr->bind();
    ret &= svr->listen();
    if (!ret)
    {
        log_error("create ws server failed");
        return false;
    }

    struct epoll_event epEvt;
    epEvt.data.fd = svr->fd;
    epEvt.events = EPOLLIN;

    int rc = epoll_ctl(epollFd, EPOLL_CTL_ADD, svr->fd, &epEvt);
    if (0 != rc)
    {
        log_error("add ws server fd to epoll failed:" << strerror(errno));
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
#ifndef MIPS
        if (-1 == num)
        {
            if (EINTR != errno)
            {
                log_error("epoll wait failed:" << strerror(errno));
                exitEpoll();
            }
        }
#else
            ef_log_error("epoll wait failed on MIPS")
#endif

        for (int i = 0; i < num; i++)
        {
            if ((waitEv[i].events & EPOLLERR) ||
                (waitEv[i].events & EPOLLHUP))
            {
                if (svr->fd == waitEv[i].data.fd)
                {
                    log_error("ws server error:" << strerror(errno));
                    exitEpoll();
                    break;
                }
                else
                {
                    unsigned int connID = waitEv[i].data.fd;
                    obj.closeNotify(connID);
                    epoll_ctl(epollFd, EPOLL_CTL_DEL, connID, NULL);
                    svr->close(connID);
                }
            }
            else if (svr->fd == waitEv[i].data.fd)
            {
                unsigned int connID = 0;
                ret = svr->accept(connID);
                if (ret)
                {
                    epEvt.data.fd = connID;
                    epEvt.events = EPOLLIN;
                    rc = epoll_ctl(epollFd, EPOLL_CTL_ADD, connID, &epEvt);
                    if (0 != rc)
                    {
                        log_error("add ws fd to epoll failed:" << strerror(errno));
                        exitEpoll();
                        break;
                    }
                    obj.connectNotify(connID);
                }
            }
            else
            {
                unsigned int connID = waitEv[i].data.fd;
                WebsocketClient *clt = dynamic_cast<WebsocketClient *>(svr->getClient(connID));
                if (NULL != clt)
                {
                    log_debug("ws epoll in");
                    std::string buf;
                    if (clt->recv(buf))
                    {
                        obj.recvNotify(connID, buf);
                    }
                }
            }
        }
    }

    return exitflag;
}

bool EpollServer::startUdpSvr(EpollServerIF &obj)
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
                log_debug("udp epoll in");
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
                log_error("invalid udp fd");
            }
        }
    }

    return exitflag;
}

}
