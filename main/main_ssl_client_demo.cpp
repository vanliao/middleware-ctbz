#include "tinylog.h"
#include "main_ssl_client_demo.h"

MAINSSLClientDemo::MAINSSLClientDemo()
{

}

MAINSSLClientDemo::~MAINSSLClientDemo()
{

}

MAINSSLClientDemo *MAINSSLClientDemo::instance()
{
    static MAINSSLClientDemo inst;
    return &inst;
}

bool MAINSSLClientDemo::start()
{
    bool ret = svr->start();
    if (!ret)
    {
        log_error("start ssl client demo failed");
        svr->stop();
        return false;
    }

    log_info("start ssl client demo succ");
    svr->connectSvr();

    return true;
}

void MAINSSLClientDemo::stop()
{
    log_info("stop client demo");
    svr->stop();
    return;
}

void MAINSSLClientDemo::loop()
{
    svr->loop();

    log_notice("ssl client demo exit");
    return;
}

bool MAINSSLClientDemo::readConfig(inifile::IniFileHelper &cfg)
{
    int ret = cfg.GetIntValue("SSLClientDemo", "port", &port);
    ret |= cfg.GetStringValue("SSLClientDemo", "server_ip", &localIP);
    ret |= cfg.GetIntValue("SSLClientDemo", "verify_peer", &verifyCA);
    ret |= cfg.GetStringValue("SSLClientDemo", "ca_file", &caFile);
    ret |= cfg.GetStringValue("SSLClientDemo", "cert_file", &certFile);
    ret |= cfg.GetStringValue("SSLClientDemo", "key_file", &keyFile);

    if (0 != ret)
    {
        log_error("read ssl client demo config failed " << ret);
        return false;
    }

    svr = std::make_shared<service::SSLClientDemo>(localIP, port);
    if (NULL == svr)
    {
        log_error("create ssl client demo failed");
        return false;
    }
    svr->setSSLCaFile(1 == verifyCA, caFile, certFile, keyFile);

    return true;
}

void MAINSSLClientDemo::addEvent(std::shared_ptr<msg::Msg> &msg)
{
    svr->addEvent(msg);
    return;
}
