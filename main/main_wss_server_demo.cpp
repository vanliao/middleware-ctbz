#include "tinylog.h"
#include "main_wss_server_demo.h"

MainWSSecServerDemo::MainWSSecServerDemo()
{

}

MainWSSecServerDemo::~MainWSSecServerDemo()
{

}

MainWSSecServerDemo *MainWSSecServerDemo::instance()
{
    static MainWSSecServerDemo inst;
    return &inst;
}

bool MainWSSecServerDemo::start()
{
    bool ret = svr->start();
    if (!ret)
    {
        log_error("start wss server demo failed");
        svr->stop();
        return false;
    }

    log_info("start wss server demo succ");

    return true;
}

void MainWSSecServerDemo::stop()
{
    log_info("stop wss server demo");
    svr->stop();
    return;
}

void MainWSSecServerDemo::loop()
{
    svr->loop();

    log_notice("wss server demo exit");
    return;
}

bool MainWSSecServerDemo::readConfig(inifile::IniFileHelper &cfg)
{
    int ret = cfg.GetIntValue("WSSServerDemo", "port", &port);
    ret |= cfg.GetStringValue("WSSServerDemo", "local_ip", &localIP);
    ret |= cfg.GetIntValue("WSSServerDemo", "verify_peer", &verifyCA);
    ret |= cfg.GetStringValue("WSSServerDemo", "ca_file", &caFile);
    ret |= cfg.GetStringValue("WSSServerDemo", "cert_file", &certFile);
    ret |= cfg.GetStringValue("WSSServerDemo", "key_file", &keyFile);

    if (0 != ret)
    {
        log_error("read wss server demo config failed");
        return false;
    }

    svr = std::make_shared<service::WSSecServerDemo>(localIP, port);

    if (NULL == svr)
    {
        log_error("create wss server demo failed");
        return false;
    }

    svr->setSSLCAFile(1 == verifyCA, caFile, certFile, keyFile);

    return true;
}

void MainWSSecServerDemo::addEvent(std::shared_ptr<msg::Msg> &msg)
{
    svr->addEvent(msg);
    return;
}
