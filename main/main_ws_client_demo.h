#ifndef MAINWSCLIENTDEMO_H
#define MAINWSCLIENTDEMO_H

#include "inifile.h"
#include "ws_client_demo.h"

class MainWSClientDemo
{
public:
    MainWSClientDemo();
    virtual ~MainWSClientDemo();
    static MainWSClientDemo *instance(void);
    virtual bool start(void);
    virtual void stop(void);
    virtual void loop(void);
    virtual bool readConfig(inifile::IniFileHelper &cfg);
    virtual void addEvent(std::shared_ptr<msg::Msg> &msg);

private:
    std::shared_ptr<service::WSClientDemo> svr;
    int port;
    std::string serverIP;
};

#endif // MAINWSCLIENTDEMO_H
