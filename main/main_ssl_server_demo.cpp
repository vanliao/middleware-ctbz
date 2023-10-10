#include "tinylog.h"
#include "main_ssl_server_demo.h"

MainSSLServerDemo::MainSSLServerDemo()
{

}

MainSSLServerDemo::~MainSSLServerDemo()
{

}

MainSSLServerDemo *MainSSLServerDemo::instance()
{
    static MainSSLServerDemo inst;
    return &inst;
}

bool MainSSLServerDemo::start()
{
    bool ret = svr->start();
    if (!ret)
    {
        log_error("start ssl server demo failed");
        svr->stop();
        return false;
    }

    log_info("start ssl server demo succ");

    return true;
}

void MainSSLServerDemo::stop()
{
    log_info("stop ssl server demo");
    svr->stop();
    return;
}

void MainSSLServerDemo::loop()
{
    svr->loop();

    log_notice("ssl server demo exit");
    return;
}

bool MainSSLServerDemo::readConfig(inifile::IniFileHelper &cfg)
{
    int ret = cfg.GetIntValue("SSLServerDemo", "port", &port);
    ret |= cfg.GetStringValue("SSLServerDemo", "local_ip", &localIP);
    ret |= cfg.GetStringValue("SSLServerDemo", "cert_file", &certFile);
    ret |= cfg.GetStringValue("SSLServerDemo", "key_file", &keyFile);

    if (0 != ret)
    {
        log_error("read server demo config failed");
        return false;
    }

    svr = std::make_shared<service::SSLServerDemo>(localIP, port);

    if (NULL == svr)
    {
        log_error("create ssl server demo failed");
        return false;
    }

    svr->setSSLPemFile(certFile, keyFile);

    return true;
}

void MainSSLServerDemo::addEvent(std::shared_ptr<msg::Msg> &msg)
{
    svr->addEvent(msg);
    return;
}
