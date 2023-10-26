#include <sys/epoll.h>
#include <string.h>
#include <functional>
#include <unistd.h>
#include <sys/eventfd.h>
#include "common_server.h"
#include "udp_server.h"
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

    log_debug("epoll create fd " << epollFd);

    return true;
}

bool CommonServer::send(const int connID, const std::string &buf)
{
    if (TCP == svrType || SSL == svrType)
    {
        network::TcpServer *svr = dynamic_cast<network::TcpServer*>(sock.get());
        if (NULL != svr)
        {
            network::TcpClient *clt = svr->getClient(connID);
            if (NULL != clt)
            {
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
            else
            {
                log_error("invalid ptr");
                return false;
            }
        }
        else
        {
            log_error("get tcp/ws server failed");
            return false;
        }
    }
    else if (WS == svrType || WSS == svrType)
    {
        network::WebsocketServer *svr = dynamic_cast<network::WebsocketServer*>(sock.get());
        if (NULL != svr)
        {
            network::WebsocketClient *clt = dynamic_cast<network::WebsocketClient *>(svr->getClient(connID));
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
            log_error("get tcp server failed");
            return false;
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
            else
            {
                log_error("invalid ptr");
                return false;
            }
        }
        else
        {
            log_error("get tcp server failed");
            return false;
        }
    }

    return true;
}

bool service::CommonServer::sendWS(const int connID, const std::string &buf, const network::WebSocket::OpCode wsOpCode)
{
    if (WS == svrType || WSS == svrType)
    {
        network::WebsocketServer *svr = dynamic_cast<network::WebsocketServer*>(sock.get());
        if (NULL != svr)
        {
            network::WebsocketClient *clt = dynamic_cast<network::WebsocketClient *>(svr->getClient(connID));
            if (NULL != clt)
            {
                clt->sendPrepare(buf, wsOpCode);
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
            log_error("get tcp server failed");
            return false;
        }
    }
    else
    {
        log_error("only send websocket msg");
        return false;
    }

    return true;
}

void CommonServer::closeDev(const unsigned int connID)
{
    if (TCP == svrType || SSL == svrType)
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
            log_error("get tcp/ws server failed");
        }
    }
    else if (WS == svrType || WSS == svrType)
    {
        network::TcpServer *svr = dynamic_cast<network::TcpServer*>(sock.get());
        if (NULL != svr)
        {
            network::WebsocketClient *clt = dynamic_cast<network::WebsocketClient *>(svr->getClient(connID));
            if (NULL != clt)
            {
                /* 发送 ws close 消息 */
                unsigned short closeCode = htobe16(1000);
                std::string buf = "";
                buf.append((char *)(&closeCode), sizeof(closeCode));
                sendWS(connID, buf, network::WebSocket::CLOSE);
                clt->closeSend();
            }
        }
        else
        {
            log_error("get tcp/ws server failed");
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
    switch (svrType)
    {
    case UDP:
    {
        ret = startUdpSvr(obj);
        break;
    }
    case TCP:
    case SSL:
    {
        ret = startTcpSvr(obj);
        break;
    }
    case WS:
    case WSS:
    {
        ret = startWSSvr(obj);
        break;
    }
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
    switch (svrType)
    {
    case UDP:
    {
        network::UdpServer *svr = dynamic_cast<network::UdpServer*>(sock.get());
        network::UdpClient *clt = svr->getClient(connID);
        return dynamic_cast<dev::EndPoint *>(clt);
    }
    case TCP:
    case SSL:
    case WS:
    case WSS:
    {
        network::TcpServer *svr = dynamic_cast<network::TcpServer*>(sock.get());
        network::TcpClient *clt = svr->getClient(connID);
        return dynamic_cast<dev::EndPoint *>(clt);
    }
    }
    return NULL;
}

dev::EndPoint *CommonServer::getFirstDev()
{
    switch (svrType)
    {
    case UDP:
    {
        network::UdpServer *svr = dynamic_cast<network::UdpServer*>(sock.get());
        network::UdpClient *clt = svr->getClient();
        return dynamic_cast<dev::EndPoint *>(clt);
    }
    case TCP:
    case SSL:
    case WS:
    case WSS:
    {
        network::TcpServer *svr = dynamic_cast<network::TcpServer*>(sock.get());
        network::TcpClient *clt = svr->getClient();
        return dynamic_cast<dev::EndPoint *>(clt);
    }
    }
    return NULL;
}

void CommonServer::getAllDev(std::vector<unsigned int> &vec)
{
    switch (svrType)
    {
    case UDP:
    {
        network::UdpServer *svr = dynamic_cast<network::UdpServer*>(sock.get());
        for (const auto &it : svr->clients)
        {
            vec.push_back(it.first);
        }
        break;
    }
    case TCP:
    case SSL:
    case WS:
    case WSS:
    {
        network::TcpServer *svr = dynamic_cast<network::TcpServer*>(sock.get());
        for (const auto &it : svr->clients)
        {
            vec.push_back(it.first);
        }
        break;
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

    auto addEpollIn = [this](unsigned int connID, int fd)->void
    {
        struct epoll_event epEvt = {0};
        epEvt.data.fd = connID;
        epEvt.events = EPOLLIN;
        int rc = epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &epEvt);
        if (unlikely(0 != rc))
        {
            log_error("add epoll in failed:" << strerror(errno));
            return;
        }
        return;
    };

    auto modifyEpollIn = [this](unsigned int connID, int fd)->void
    {
        struct epoll_event epEvt = {0};
        epEvt.data.fd = connID;
        epEvt.events = EPOLLIN;
        int rc = epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &epEvt);
        if (unlikely(0 != rc))
        {
            log_error("mod epoll in failed:" << strerror(errno));
            return;
        }
        return;
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
            else if (waitEv[i].events & EPOLLOUT)
            {
                log_debug("epoll out");
                unsigned int connID = waitEv[i].data.fd;
                network::TcpClient *clt = svr->getClient(connID);
                if (likely((NULL != clt)))
                {
                    network::Socket::POLL_RESULT pr = clt->pollOut();
                    switch (pr)
                    {
                    /* 发送数据成功 */
                    case network::Socket::POLL_RESULT::POLL_RESULT_SUCCESS:
                    {
                        modifyEpollIn(connID, clt->fd);
                        break;
                    }
                    case network::Socket::POLL_RESULT::POLL_RESULT_READ:
                    case network::Socket::POLL_RESULT::POLL_RESULT_SEND:
                    case network::Socket::POLL_RESULT::POLL_RESULT_HANDSHAKE:
                    {
                        break;
                    }
                    /* 出错 关闭连接 */
                    case network::Socket::POLL_RESULT::POLL_RESULT_CLOSE:
                    {
                        log_error("epoll out failed");
                        obj.closeNotify(connID);
                        epoll_ctl(epollFd, EPOLL_CTL_DEL, clt->fd, NULL);
                        svr->close(connID);
                        break;
                    }
                    default:
                    {
                        log_error("invalid poll result:" << pr);
                        break;
                    }
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
                    addEpollIn(connID, svr->getClient(connID)->fd);
                    obj.connectNotify(connID);
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
                    if (likely(NULL != clt))
                    {
                        network::Socket::POLL_RESULT pr = clt->pollIn();
                        switch (pr)
                        {
                        /* 接收数据成功 */
                        case network::Socket::POLL_RESULT::POLL_RESULT_SUCCESS:
                        {
                            obj.recvNotify(connID, clt->recvBuf);
                            clt->recvBuf = "";
                            break;
                        }
                        case network::Socket::POLL_RESULT::POLL_RESULT_READ:
                        case network::Socket::POLL_RESULT::POLL_RESULT_SEND:
                        case network::Socket::POLL_RESULT::POLL_RESULT_HANDSHAKE:
                        {
                            break;
                        }
                        /* 出错 关闭连接 */
                        case network::Socket::POLL_RESULT::POLL_RESULT_CLOSE:
                        {
                            obj.closeNotify(connID);
                            epoll_ctl(epollFd, EPOLL_CTL_DEL, clt->fd, NULL);
                            svr->close(connID);
                            break;
                        }
                        default:
                        {
                            log_error("invalid poll result:" << pr);
                            break;
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

bool CommonServer::startWSSvr(CommonServerIF &obj)
{
    network::WebsocketServer *svr = dynamic_cast<network::WebsocketServer*>(sock.get());
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

    auto modifyEpollIn = [this](unsigned int connID, int fd)->void
    {
        struct epoll_event epEvt = {0};
        epEvt.data.fd = connID;
        epEvt.events = EPOLLIN;
        int rc = epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &epEvt);
        if (unlikely(0 != rc))
        {
            log_error("mod epoll in failed:" << strerror(errno));
            return;
        }
        return;
    };

    auto modifyEpollOut = [this](unsigned int connID, int fd)->void
    {
        struct epoll_event epEvt = {0};
        epEvt.data.fd = connID;
        epEvt.events = EPOLLIN|EPOLLOUT;
        int rc = epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &epEvt);
        if (unlikely(0 != rc))
        {
            log_error("mod epoll out failed:" << strerror(errno));
            return;
        }
        return;
    };

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
                log_debug("ws epoll err");
                if (svr->fd == waitEv[i].data.fd)
                {
                    log_error("ws server error:" << strerror(errno));
                    exitEpoll();
                    break;
                }
                else
                {
                    unsigned int connID = waitEv[i].data.fd;
                    network::WebsocketClient *clt = dynamic_cast<network::WebsocketClient *>(svr->getClient(connID));
                    if (NULL != clt)
                    {
                        obj.closeNotify(connID);
                        epoll_ctl(epollFd, EPOLL_CTL_DEL, clt->fd, NULL);
                        svr->close(connID);
                    }
                }
            }
            else if (waitEv[i].events & EPOLLOUT)
            {
                log_debug("ws epoll out");
                unsigned int connID = waitEv[i].data.fd;
                network::WebsocketClient *clt = dynamic_cast<network::WebsocketClient *>(svr->getClient(connID));
                if (NULL != clt)
                {
                    network::Socket::POLL_RESULT pr = clt->pollOut();
                    switch (pr)
                    {
                    /* 发送数据成功 */
                    case network::Socket::POLL_RESULT::POLL_RESULT_SUCCESS:
                    {
                        modifyEpollIn(connID, clt->fd);
                        break;
                    }
                    case network::Socket::POLL_RESULT::POLL_RESULT_READ:
                    case network::Socket::POLL_RESULT::POLL_RESULT_SEND:
                    {
                        break;
                    }
                    case network::Socket::POLL_RESULT::POLL_RESULT_HANDSHAKE:
                    {
                        break;
                    }
                    /* 出错 关闭连接 */
                    case network::Socket::POLL_RESULT::POLL_RESULT_CLOSE:
                    {
                        log_debug("epoll out close connection");
                        obj.closeNotify(connID);
                        epoll_ctl(epollFd, EPOLL_CTL_DEL, clt->fd, NULL);
                        svr->close(connID);
                        break;
                    }
                    default:
                    {
                        log_error("invalid poll result:" << pr);
                        break;
                    }
                    }
                }
            }
            else if (svr->fd == waitEv[i].data.fd)
            {
                log_debug("ws epoll accept");
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
                }
            }
            else
            {
                log_debug("ws epoll in");
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
                    network::WebsocketClient *clt = dynamic_cast<network::WebsocketClient *>(svr->getClient(connID));
                    if (likely(NULL != clt))
                    {
                        bool flag = true;
                        do
                        {
                            network::Socket::POLL_RESULT pr = clt->pollIn();
                            switch (pr)
                            {
                            /* 接收数据成功 */
                            case network::Socket::POLL_RESULT::POLL_RESULT_SUCCESS:
                            {
                                obj.recvNotify(connID, clt->readyWSFrame);
                                clt->readyWSFrame = "";
                                if (clt->recvBuf.empty())
                                {
                                    flag = false;
                                }
                                break;
                            }
                            case network::Socket::POLL_RESULT::POLL_RESULT_READ:
                            {
                                flag = false;
                                break;
                            }
                            case network::Socket::POLL_RESULT::POLL_RESULT_SEND:
                            {
                                modifyEpollOut(connID, clt->fd);
                                if (clt->recvBuf.empty())
                                {
                                    flag = false;
                                }
                                break;
                            }
                            case network::Socket::POLL_RESULT::POLL_RESULT_HANDSHAKE:
                            {
                                /* WS 连接成功 */
                                obj.connectNotify(connID);
                                if (clt->recvBuf.empty())
                                {
                                    flag = false;
                                }
                                break;
                            }
                            /* 出错 关闭连接 */
                            case network::Socket::POLL_RESULT::POLL_RESULT_CLOSE:
                            {
                                log_debug("epoll in close connection");
                                obj.closeNotify(connID);
                                epoll_ctl(epollFd, EPOLL_CTL_DEL, clt->fd, NULL);
                                svr->close(connID);
                                flag = false;
                                break;
                            }
                            default:
                            {
                                log_error("invalid poll result:" << pr);
                                flag = false;
                                break;
                            }
                            }
                        }while(flag);
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
            else if (waitEv[i].events & EPOLLOUT)
            {
                log_debug("epoll out");
                unsigned int connID = waitEv[i].data.fd;
                network::UdpClient *clt = svr->getClient(connID);
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
                                struct epoll_event epEvt;
                                epEvt.data.fd = clt->fd;    //EPOLLIN的时候只能设置UDP服务器的FD
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
    return model.stop();
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

void Server::initExternEvent()
{
    model.addExtenEvent(notifyEvt);
}

void Server::run()
{
    notifyEvt = eventfd(0, 0);
    if (-1 == notifyEvt)
    {
        log_error("create event failed:" << strerror(errno));
        return;
    }

    log_debug("event create fd " << notifyEvt);

    bool ret = model.open();
    if (ret)
    {
        initExternEvent();
        model.start(*this);
    }

    return;
}

}
