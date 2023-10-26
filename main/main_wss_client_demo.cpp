#include "main_wss_client_demo.h"
#include "tinylog.h"

MainWSSecClientDemo::MainWSSecClientDemo()
{

}

MainWSSecClientDemo::~MainWSSecClientDemo()
{

}

MainWSSecClientDemo *MainWSSecClientDemo::instance()
{
    static MainWSSecClientDemo inst;
    return &inst;
}

bool MainWSSecClientDemo::start()
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

void MainWSSecClientDemo::stop()
{
    log_info("stop ws client demo");
    svr->stop();
    return;
}

void MainWSSecClientDemo::loop()
{
    svr->loop();

    log_notice("ws client demo exit");
    return;
}

bool MainWSSecClientDemo::readConfig(inifile::IniFileHelper &cfg)
{
    int ret = cfg.GetIntValue("WSSClientDemo", "port", &port);
    ret |= cfg.GetStringValue("WSSClientDemo", "server_ip", &serverIP);
    ret |= cfg.GetIntValue("WSSServerDemo", "verify_peer", &verifyCA);
    ret |= cfg.GetStringValue("WSSServerDemo", "ca_file", &caFile);
    ret |= cfg.GetStringValue("WSSServerDemo", "cert_file", &certFile);
    ret |= cfg.GetStringValue("WSSServerDemo", "key_file", &keyFile);

    if (0 != ret)
    {
        log_error("read wss client demo config failed");
        return false;
    }

    svr = std::make_shared<service::WSSecClientDemo>(serverIP, port);
    if (NULL == svr)
    {
        log_error("create wss client demo failed");
        return false;
    }
    svr->setSSLCaFile(1 == verifyCA, caFile, certFile, keyFile);

    return true;
}

void MainWSSecClientDemo::addEvent(std::shared_ptr<msg::Msg> &msg)
{
    svr->addEvent(msg);
    return;
}
