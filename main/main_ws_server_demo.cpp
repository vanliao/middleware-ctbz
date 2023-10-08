#include "tinylog.h"
#include "main_ws_server_demo.h"

MainWSServerDemo::MainWSServerDemo()
{

}

MainWSServerDemo::~MainWSServerDemo()
{

}

MainWSServerDemo *MainWSServerDemo::instance()
{
    static MainWSServerDemo inst;
    return &inst;
}

bool MainWSServerDemo::start()
{
    bool ret = svr->start();
    if (!ret)
    {
        log_error("start ws server demo failed");
        svr->stop();
        return false;
    }

    log_info("start ws server demo succ");

    return true;
}

void MainWSServerDemo::stop()
{
    log_info("stop server demo");
    svr->stop();
    return;
}

void MainWSServerDemo::loop()
{
    svr->loop();

    log_notice("ws server demo exit");
    return;
}

bool MainWSServerDemo::readConfig(inifile::IniFileHelper &cfg)
{
    int ret = cfg.GetIntValue("WSServerDemo", "port", &port);
    ret |= cfg.GetStringValue("WSServerDemo", "local_ip", &localIP);

    if (0 != ret)
    {
        log_error("read ws server demo config failed");
        return false;
    }

    svr = std::make_shared<service::WSServerDemo>(localIP, port);

    if (NULL == svr)
    {
        log_error("create ws server demo failed");
        return false;
    }

    return true;
}

void MainWSServerDemo::addEvent(std::shared_ptr<msg::Msg> &msg)
{
    svr->addEvent(msg);
    return;
}
