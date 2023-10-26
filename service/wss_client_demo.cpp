#include "wss_client_demo.h"
#include <unistd.h>
#include "ws_dev_demo.h"
#include "tinylog.h"

namespace service {

WSSecClientDemo::WSSecClientDemo(const std::string &serverIP, const int serverPort) :
    Communicator(serverIP, serverPort, network::EpollCommunicator::WSS)
{
    return;
}

WSSecClientDemo::~WSSecClientDemo()
{
    return;
}

void WSSecClientDemo::connectNotify(unsigned int connID)
{
    log_info("connectNotify:" << connID);
    dev::EndPoint *ep = model.getDev(connID);
    if (NULL == ep)
    {
        log_fatal("DEV invalid ptr");
        return;
    }
    ep->dev = std::make_shared<dev::WSDevDemo>();

    return;
}

void WSSecClientDemo::recvNotify(unsigned int connID, std::string &buf)
{
    log_info("recvNotify:" << connID << "[" << buf.length() << "]" << buf);
    dev::EndPoint *ep = model.getDev(connID);
    if (NULL == ep)
    {
        log_fatal("invalid ptr");
        return;
    }

//    model.sendWS(connID, "", network::WebSocket::PING);
//    {
//    static int count = 0;
//    count++;
//    if (0 == count % 5)
//        model.disconnect(connID);
//    }

    dev::WSDevDemo *dev = dynamic_cast<dev::WSDevDemo *>(ep->dev.get());
    if (dev)
    {
        std::shared_ptr<msg::Msg> msg = std::make_shared<msg::Msg>();
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

void WSSecClientDemo::closeNotify(unsigned int connID)
{
    log_info("closeNotify:" << connID);
    return;
}

void WSSecClientDemo::eventtNotify(const int event)
{
    if (event == notifyEvt)
    {
        procEvent();
    }
    else if (event == hbTimer)
    {
        procHbTimer();

        {
            dev::EndPoint *ep = model.getFirstDev();
            if (NULL != ep && NULL != ep->dev)
            {
                model.sendWS(ep->connID, "{\"msgType\":\"echo\", \"data\":\"hello\"}", network::WebSocket::TEXT);
            }

            static int cnt = 0;
            ++cnt;
            if (3 == cnt)
            {
                log_debug("wss client close connection " << ep->connID);
                cnt = 0;
                model.disconnect(ep->connID);
            }
        }
    }
    else
    {
        log_error("ivnalid event fd " << event);
    }

    return;
}

void WSSecClientDemo::initExternEvent()
{
    Communicator::initExternEvent();
    return;
}

void WSSecClientDemo::setSSLCaFile(const bool verifyPeer,
                                   const std::string &caFile,
                                   const std::string &certFile,
                                   const std::string &keyFile)
{
    model.setSSLCAFile(verifyPeer, caFile, certFile, keyFile);
}

void WSSecClientDemo::procEvent()
{
    uint64_t buf;
    read(notifyEvt, &buf, sizeof(uint64_t));
    log_debug("eventtNotify:" << notifyEvt << " " << buf);

    std::list<std::shared_ptr<msg::Msg> > msgs;
    {
    std::lock_guard<std::mutex> locker(mu);
    msgList.swap(msgs);
    }

    while (!msgs.empty())
    {
        std::shared_ptr<msg::Msg> reqMsg = msgs.front();
        msgs.pop_front();

        if (!reqMsg->verify())
        {
            log_error("get invalid msg " << reqMsg->type);
            continue;
        }

        std::stringstream ss;
        reqMsg->show(ss);

        dev::EndPoint *ep = model.getFirstDev();
        if (NULL != ep)
        {
            if (NULL != ep->dev)
            {
                std::shared_ptr<msg::Msg> rspMsg;
                auto pr = ep->dev->procEvent(reqMsg, rspMsg);
                procDevResult(pr, ep, reqMsg, rspMsg);
                log_info("proc event finish");
            }
            else
            {
                log_fatal("invalid ptr");
            }
        }

    }

    return;
}

void WSSecClientDemo::procDevResult(dev::Dev::ProcResult pr,
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
        model.sendWS(ep->connID, rspMsg->raw, network::WebSocket::TEXT);
        log_info("send to wss demo:" << rspMsg->raw);
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
