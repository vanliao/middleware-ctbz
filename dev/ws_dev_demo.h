#ifndef WSDEVDEMO_H
#define WSDEVDEMO_H

#include <map>
#include <functional>
#include "dev.h"
#include "document.h"

namespace dev {

class WSDevDemo : public Dev
{
public:
    WSDevDemo();
    virtual ~WSDevDemo();
    ProcResult procEvent(const std::shared_ptr<msg::Msg> &msg, std::shared_ptr<msg::Msg> &outMsg);
    virtual ProcResult recv(const std::string &data, std::shared_ptr<msg::Msg> &outMsg);

private:
    ProcResult procDevMsg(rapidjson::Document &doc, std::shared_ptr<msg::Msg> &outMsg);
    ProcResult procMsgLogin(const rapidjson::Document &doc, std::shared_ptr<msg::Msg> &outMsg);
    ProcResult procMsgEcho(const rapidjson::Document &doc, std::shared_ptr<msg::Msg> &outMsg);
    ProcResult procEventDemo(const std::shared_ptr<msg::Msg> &msg, std::shared_ptr<msg::Msg> &outMsg);

private:
    bool isLogin;

private:
    std::map<std::string, std::function<Dev::ProcResult(const rapidjson::Document &doc, std::shared_ptr<msg::Msg> &outMsg)> > msgCallback;
    std::map<int, std::function<Dev::ProcResult(const std::shared_ptr<msg::Msg> &msg, std::shared_ptr<msg::Msg> &outMsg)> > evCallback;
};

}
#endif // WSDEVDEMO_H
