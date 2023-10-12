#ifndef MAINSSLSERVERDEMO_H
#define MAINSSLSERVERDEMO_H


class SSLServerDemo
{
public:
    SSLServerDemo();
};

#include "inifile.h"
#include "ssl_server_demo.h"

class MainSSLServerDemo
{
public:
    MainSSLServerDemo();
    virtual ~MainSSLServerDemo();
    static MainSSLServerDemo *instance(void);
    virtual bool start(void);
    virtual void stop(void);
    virtual void loop(void);
    virtual bool readConfig(inifile::IniFileHelper &cfg);
    virtual void addEvent(std::shared_ptr<msg::Msg> &msg);

private:
    std::shared_ptr<service::SSLServerDemo> svr;
    int port;
    int verifyCA;
    std::string localIP;
    std::string caFile;
    std::string certFile;
    std::string keyFile;
};

#endif // MAINSSLSERVERDEMO_H
