#ifndef MAINWSSECSERVERDEMO_H
#define MAINWSSECSERVERDEMO_H

#include "inifile.h"
#include "wss_server_demo.h"

class MainWSSecServerDemo
{
public:
    MainWSSecServerDemo();
    virtual ~MainWSSecServerDemo();
    static MainWSSecServerDemo *instance(void);
    virtual bool start(void);
    virtual void stop(void);
    virtual void loop(void);
    virtual bool readConfig(inifile::IniFileHelper &cfg);
    virtual void addEvent(std::shared_ptr<msg::Msg> &msg);

private:
    std::shared_ptr<service::WSSecServerDemo> svr;
    int port;
    std::string localIP;
    int verifyCA;
    std::string caFile;
    std::string certFile;
    std::string keyFile;
};

#endif // MAINWSSECSERVERDEMO_H
