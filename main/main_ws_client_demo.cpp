#include "main_ws_client_demo.h"
#include "tinylog.h"

MainWSClientDemo::MainWSClientDemo()
{

}

MainWSClientDemo::~MainWSClientDemo()
{

}

MainWSClientDemo *MainWSClientDemo::instance()
{
    static MainWSClientDemo inst;
    return &inst;
}

bool MainWSClientDemo::start()
{
    bool ret = svr->start();
    if (!ret)
    {
        log_error("start ws client demo failed");
        svr->stop();
        return false;
    }

    log_info("start ws client demo succ");
    svr->connectSvr();

    return true;
}

void MainWSClientDemo::stop()
{
    log_info("stop ws client demo");
    svr->stop();
    return;
}

void MainWSClientDemo::loop()
{
    svr->loop();

    log_notice("ws client demo exit");
    return;
}

bool MainWSClientDemo::readConfig(inifile::IniFileHelper &cfg)
{
    int ret = cfg.GetIntValue("WSClientDemo", "port", &port);
    ret |= cfg.GetStringValue("WSClientDemo", "server_ip", &serverIP);

    if (0 != ret)
    {
        log_error("read ws client demo config failed");
        return false;
    }

    svr = std::make_shared<service::WSClientDemo>(serverIP, port);
    if (NULL == svr)
    {
        log_error("create ws client demo failed");
        return false;
    }

    return true;
}

void MainWSClientDemo::addEvent(std::shared_ptr<msg::Msg> &msg)
{
    svr->addEvent(msg);
    return;
}
