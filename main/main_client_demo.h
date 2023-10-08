#ifndef MAINCLIENTDEMO_H
#define MAINCLIENTDEMO_H

#include "inifile.h"
#include "client_demo.h"

class MainClientDemo
{
public:
    MainClientDemo();
    virtual ~MainClientDemo();
    static MainClientDemo *instance(void);
    virtual bool start(void);
    virtual void stop(void);
    virtual void loop(void);
    virtual bool readConfig(inifile::IniFileHelper &cfg);
    virtual void addEvent(std::shared_ptr<msg::Msg> &msg);

private:
    std::shared_ptr<service::ClientDemo> svr;
    int port;
    std::string localIP;
};

#endif // MAINCLIENTDEMO_H
