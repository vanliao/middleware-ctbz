#include <unistd.h>
#include "tinylog.h"
#include "lte_dev_demo.h"
#include "ssl_client_demo.h"

namespace service {

SSLClientDemo::SSLClientDemo(const std::string &serverIP, const int serverPort):
    Communicator(serverIP, serverPort, network::EpollCommunicator::SSL)
{
    return;
}

SSLClientDemo::~SSLClientDemo()
{
    return;
}

void SSLClientDemo::connectNotify(unsigned int connID)
{
    log_info("connectNotify:" << connID);
    dev::EndPoint *ep = model.getDev(connID);
    if (NULL == ep)
    {
        log_fatal("invalid ptr");
        return;
    }
    ep->dev = std::make_shared<dev::LteDevDemo>();
    model.send(connID, "{\"code\":1,\"data\":\"123456789\"}");
}

void SSLClientDemo::recvNotify(unsigned int connID, std::string &buf)
{
    log_info("recvNotify:" << connID << "[" << buf.length() << "]" << buf);
    dev::EndPoint *ep = model.getDev(connID);
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
//    model.disconnect(ep->connID);
    return;
}

void SSLClientDemo::closeNotify(unsigned int connID)
{
    log_info("closeNotify:" << connID);
    return;
}

void SSLClientDemo::eventtNotify(const int event)
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

void SSLClientDemo::initExternEvent()
{
    Communicator::initExternEvent();
    return;
}

void SSLClientDemo::setSSLCaFile(const bool verifyPeer,
                                 const std::string &caFile,
                                 const std::string &certFile,
                                 const std::string &keyFile)
{
    model.setSSLCAFile(verifyPeer, caFile, certFile, keyFile);
}

void SSLClientDemo::procEvent()
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
            log_error("get invalid msg");
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

void SSLClientDemo::procDevResult(dev::Dev::ProcResult pr,
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
        model.send(ep->connID, rspMsg->raw);
        log_info("send to ssl demo:" << rspMsg->raw);
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
