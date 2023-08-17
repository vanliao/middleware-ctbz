#include "tinylog.h"
#include "oam_cgi_server_demo.h"

OamCgiServerDemo::OamCgiServerDemo()
{

}

OamCgiServerDemo::~OamCgiServerDemo()
{

}

OamCgiServerDemo *OamCgiServerDemo::instance()
{
    static OamCgiServerDemo inst;
    return &inst;
}

bool OamCgiServerDemo::start()
{
    bool ret = svr->start();
    if (!ret)
    {
        log_error("start server demo failed");
        svr->stop();
        return false;
    }

    log_info("start server demo succ");

    return true;
}

void OamCgiServerDemo::stop()
{
    log_info("stop server demo");
    svr->stop();
    return;
}

void OamCgiServerDemo::loop()
{
    svr->loop();

    log_notice("server demo exit");
    return;
}

bool OamCgiServerDemo::readConfig(inifile::IniFileHelper &cfg)
{
    int ret = cfg.GetIntValue("ServerDemo", "port", &port);
    ret |= cfg.GetStringValue("ServerDemo", "local_ip", &localIP);

    if (0 != ret)
    {
        log_error("read server demo config failed");
        return false;
    }

    svr = std::make_shared<service::ServerDemo>(localIP, port);

    if (NULL == svr)
    {
        log_error("create server demo failed");
        return false;
    }

    return true;
}

void OamCgiServerDemo::addEvent(std::shared_ptr<msg::Msg> &msg)
{
    svr->addEvent(msg);
    return;
}
