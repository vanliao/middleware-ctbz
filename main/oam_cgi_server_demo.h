#ifndef OAMCGINR_H
#define OAMCGINR_H

#include "inifile.h"
#include "server_demo.h"

class OamCgiServerDemo
{
public:
    OamCgiServerDemo();
    virtual ~OamCgiServerDemo();
    static OamCgiServerDemo *instance(void);
    virtual bool start(void);
    virtual void stop(void);
    virtual void loop(void);
    virtual bool readConfig(inifile::IniFileHelper &cfg);
    virtual void addEvent(std::shared_ptr<msg::Msg> &msg);

private:
    std::shared_ptr<service::ServerDemo> svr;
    int port;
    std::string localIP;
};

#endif // OAMCGINR_H
