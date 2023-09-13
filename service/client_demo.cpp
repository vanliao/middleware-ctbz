#include <unistd.h>
#include "tinylog.h"
#include "client_demo.h"
#include "lte_dev_demo.h"

namespace service {

ClientDemo::ClientDemo(const std::string &serverIP, const int serverPort):
    CommonCommunicator(serverIP, serverPort, network::EpollCommunicator::UDP)
{
    return;
}

ClientDemo::~ClientDemo()
{
    return;
}

void ClientDemo::connectNotify(unsigned int connID)
{
    log_info("connectNotify:" << connID);
    dev::EndPoint *ep = svr.getDev(connID);
    if (NULL == ep)
    {
        log_fatal("invalid ptr");
        return;
    }
    ep->dev = std::make_shared<dev::LteDevDemo>();
    ep->send("{\"code\":20,\"data\":\"123456789\"}");
}

void ClientDemo::recvNotify(unsigned int connID, std::string &buf)
{
    log_info("recvNotify:" << connID << "[" << buf.length() << "]" << buf);
    dev::EndPoint *ep = svr.getDev(connID);
    if (NULL == ep)
    {
        log_fatal("invalid ptr");
        return;
    }

    dev::LteDevDemo *dev = dynamic_cast<dev::LteDevDemo *>(ep->dev.get());
    if (dev)
    {
        std::shared_ptr<msg::Msg> msg = std::make_shared<msg::Msg>();
        msg->src = msg::src::SRC_CGI;
        msg->raw = buf;
        while (1)
        {
            std::shared_ptr<msg::Msg> outMsg;
            auto pr = dev->recv(buf, outMsg);
            procDevResult(pr, ep, msg, outMsg);
            if (dev::Dev::RECVMORE == pr)
            {
                break;
            }
            buf = "";
        }
    }
    else
    {
        log_fatal("invalid ptr");
        return;
    }
    return;
}

void service::ClientDemo::closeNotify(unsigned int connID)
{
    log_info("closeNotify:" << connID);
    return;
}

void service::ClientDemo::eventtNotify(const int event)
{
    if (event == notifyEvt)
    {
        procEvent();
    }
    else if (event == hbTimer)
    {
        procHbTimer();
    }
    else
    {
        log_error("ivnalid event fd " << event);
    }

    return;
}

void ClientDemo::procEvent()
{
    std::list<std::shared_ptr<msg::Msg> > msgs;
    {
    std::lock_guard<std::mutex> locker(mu);
    uint64_t buf;
    read(notifyEvt, &buf, sizeof(uint64_t));
    log_debug("eventtNotify:" << notifyEvt << " " << buf);
    msgList.swap(msgs);
    }

    while (!msgs.empty())
    {
        std::shared_ptr<msg::Msg> reqMsg = msgs.front();
        msgs.pop_front();

        if (!reqMsg->verify())
        {
            log_error("get invalid msg");
            continue;
        }

        std::stringstream ss;
        reqMsg->show(ss);

        dev::EndPoint *ep = svr.getFirstDev();
        if (NULL != ep)
        {
            if (NULL != ep->dev)
            {
                std::shared_ptr<msg::Msg> rspMsg;
                auto pr = ep->dev->procEvent(reqMsg, rspMsg);
                procDevResult(pr, ep, reqMsg, rspMsg);
                log_debug("proc event finish");
            }
            else
            {
                log_fatal("invalid ptr");
            }
        }

    }

    return;
}

void ClientDemo::procDevResult(dev::Dev::ProcResult pr,
                               dev::EndPoint *ep,
                               std::shared_ptr<msg::Msg> &reqMsg,
                               std::shared_ptr<msg::Msg> &rspMsg)
{
    switch (pr)
    {
    case dev::Dev::NONE:
    {
        break;
    }
    case dev::Dev::SENDTODEV:
    {
        ep->send(rspMsg->raw);
        log_info("send to client demo:" << rspMsg->raw);
        break;
    }
    default:
    {
        break;
    }
    }
    return;
}
}
