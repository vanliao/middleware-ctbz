#ifndef OAMCGICLIENTDEMO_H
#define OAMCGICLIENTDEMO_H

#include "inifile.h"
#include "client_demo.h"

class OamCgiClientDemo
{
public:
    OamCgiClientDemo();
    virtual ~OamCgiClientDemo();
    static OamCgiClientDemo *instance(void);
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

#endif // OAMCGICLIENTDEMO_H
