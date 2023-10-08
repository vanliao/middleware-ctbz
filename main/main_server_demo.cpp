#include "tinylog.h"
#include "main_server_demo.h"

MainServerDemo::MainServerDemo()
{

}

MainServerDemo::~MainServerDemo()
{

}

MainServerDemo *MainServerDemo::instance()
{
    static MainServerDemo inst;
    return &inst;
}

bool MainServerDemo::start()
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

void MainServerDemo::stop()
{
    log_info("stop server demo");
    svr->stop();
    return;
}

void MainServerDemo::loop()
{
    svr->loop();

    log_notice("server demo exit");
    return;
}

bool MainServerDemo::readConfig(inifile::IniFileHelper &cfg)
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

void MainServerDemo::addEvent(std::shared_ptr<msg::Msg> &msg)
{
    svr->addEvent(msg);
    return;
}
