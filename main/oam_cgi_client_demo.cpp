#include "tinylog.h"
#include "oam_cgi_client_demo.h"

OamCgiClientDemo::OamCgiClientDemo()
{

}

OamCgiClientDemo::~OamCgiClientDemo()
{

}

OamCgiClientDemo *OamCgiClientDemo::instance()
{
    static OamCgiClientDemo inst;
    return &inst;
}

bool OamCgiClientDemo::start()
{
    bool ret = svr->start();
    if (!ret)
    {
        log_error("start client demo failed");
        svr->stop();
        return false;
    }

    log_info("start client demo succ");
    svr->connectSvr();

    return true;
}

void OamCgiClientDemo::stop()
{
    log_info("stop client demo");
    svr->stop();
    return;
}

void OamCgiClientDemo::loop()
{
    svr->loop();

    log_notice("client demo exit");
    return;
}

bool OamCgiClientDemo::readConfig(inifile::IniFileHelper &cfg)
{
    int ret = cfg.GetIntValue("ClientDemo", "port", &port);
    ret |= cfg.GetStringValue("ClientDemo", "server_ip", &localIP);

    if (0 != ret)
    {
        log_error("read client demo config failed " << ret);
        return false;
    }

    svr = std::make_shared<service::ClientDemo>(localIP, port);
    if (NULL == svr)
    {
        log_error("create client demo failed");
        return false;
    }

    return true;
}

void OamCgiClientDemo::addEvent(std::shared_ptr<msg::Msg> &msg)
{
    svr->addEvent(msg);
    return;
}
