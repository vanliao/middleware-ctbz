#ifndef MAINWSSERVERDEMO_H
#define MAINWSSERVERDEMO_H

#include "inifile.h"
#include "ws_server_demo.h"

class MainWSServerDemo
{
public:
    MainWSServerDemo();
    virtual ~MainWSServerDemo();
    static MainWSServerDemo *instance(void);
    virtual bool start(void);
    virtual void stop(void);
    virtual void loop(void);
    virtual bool readConfig(inifile::IniFileHelper &cfg);
    virtual void addEvent(std::shared_ptr<msg::Msg> &msg);

private:
    std::shared_ptr<service::WSServerDemo> svr;
    int port;
    std::string localIP;
};

#endif // MAINWSSERVERDEMO_H
