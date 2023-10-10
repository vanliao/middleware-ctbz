#ifndef MAINSSLCLIENTDEMO_H
#define MAINSSLCLIENTDEMO_H

#include "inifile.h"
#include "ssl_client_demo.h"

class MAINSSLClientDemo
{
public:
    MAINSSLClientDemo();
    virtual ~MAINSSLClientDemo();
    static MAINSSLClientDemo *instance(void);
    virtual bool start(void);
    virtual void stop(void);
    virtual void loop(void);
    virtual bool readConfig(inifile::IniFileHelper &cfg);
    virtual void addEvent(std::shared_ptr<msg::Msg> &msg);

private:
    std::shared_ptr<service::SSLClientDemo> svr;
    int port;
    std::string localIP;
};

#endif // MAINSSLCLIENTDEMO_H
