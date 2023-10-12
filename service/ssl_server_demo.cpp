#include <unistd.h>
#include "ssl_server_demo.h"
#include "nr_dev_demo.h"
#include "tinylog.h"

namespace service {

SSLServerDemo::SSLServerDemo(const std::string &serverIP, const int serverPort) :
    Server(serverIP, serverPort, network::EpollServer::SSL)
{
    return;
}

SSLServerDemo::~SSLServerDemo()
{
    return;
}

void SSLServerDemo::connectNotify(unsigned int connID)
{
    log_info("connectNotify:" << connID);
    dev::EndPoint *ep = model.getDev(connID);
    if (NULL == ep)
    {
        log_fatal("DEV invalid ptr");
        return;
    }
    ep->dev = std::make_shared<dev::NrDevDemo>();
    return;
}

void SSLServerDemo::recvNotify(unsigned int connID, std::string &buf)
{
    log_info("recvNotify:" << connID << "[" << buf.length() << "]" << buf);
    dev::EndPoint *ep = model.getDev(connID);
    if (NULL == ep)
    {
        log_fatal("invalid ptr");
        return;
    }

    dev::NrDevDemo *dev = dynamic_cast<dev::NrDevDemo *>(ep->dev.get());
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

//    model.closeDev(ep->connID);

    return;
}

void SSLServerDemo::closeNotify(unsigned int connID)
{
    log_info("closeNotify:" << connID);
    return;
}

void SSLServerDemo::eventtNotify(const int event)
{
    if (event == notifyEvt)
    {
        procEvent();
    }
    else
    {
        log_error("ivnalid evnet fd");
    }

    return;
}

void SSLServerDemo::initExternEvent()
{
    Server::initExternEvent();
    return;
}

void SSLServerDemo::setSSLCAFile(const bool verifyPeer,
                                 const std::string &caFilePath,
                                 const std::string &certFilePath,
                                 const std::string &keyFilePath)
{
    model.setSSLCAFile(verifyPeer, caFilePath, certFilePath, keyFilePath);
    return;
}

void SSLServerDemo::procEvent()
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

void SSLServerDemo::procDevResult(dev::Dev::ProcResult pr,
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
        log_info("send to server demo:" << rspMsg->raw);
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
