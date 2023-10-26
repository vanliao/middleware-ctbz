#ifndef MAINWSSECCLIENTDEMO_H
#define MAINWSSECCLIENTDEMO_H

#include "inifile.h"
#include "wss_client_demo.h"

class MainWSSecClientDemo
{
public:
    MainWSSecClientDemo();
    virtual ~MainWSSecClientDemo();
    static MainWSSecClientDemo *instance(void);
    virtual bool start(void);
    virtual void stop(void);
    virtual void loop(void);
    virtual bool readConfig(inifile::IniFileHelper &cfg);
    virtual void addEvent(std::shared_ptr<msg::Msg> &msg);

private:
    std::shared_ptr<service::WSSecClientDemo> svr;
    int port;
    std::string serverIP;
    int verifyCA;
    std::string caFile;
    std::string certFile;
    std::string keyFile;
};

#endif // MAINWSSECCLIENTDEMO_H
