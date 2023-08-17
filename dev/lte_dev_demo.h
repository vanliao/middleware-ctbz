#ifndef LteDevDemo_H
#define LteDevDemo_H

#include <map>
#include <functional>
#include "dev.h"
#include "document.h"

namespace dev {

class LteDevDemo: public Dev
{
public:
    LteDevDemo();
    virtual ~LteDevDemo();
    ProcResult procEvent(const std::shared_ptr<msg::Msg> &msg, std::shared_ptr<msg::Msg> &outMsg);
    virtual ProcResult recv(const std::string &data, std::shared_ptr<msg::Msg> &outMsg);

private:
    ProcResult procDevMsg(rapidjson::Document &doc, std::shared_ptr<msg::Msg> &outMsg);
    ProcResult procMsgDemo(const rapidjson::Document &doc, std::shared_ptr<msg::Msg> &outMsg);
    ProcResult procEventDemo(const std::shared_ptr<msg::Msg> &msg, std::shared_ptr<msg::Msg> &outMsg);

private:
    unsigned int headerLen;
    std::map<int, std::function<Dev::ProcResult(const rapidjson::Document &doc, std::shared_ptr<msg::Msg> &outMsg)> > msgCallback;
    std::map<int, std::function<Dev::ProcResult(const std::shared_ptr<msg::Msg> &msg, std::shared_ptr<msg::Msg> &outMsg)> > evCallback;
};

}
#endif // LteDevDemo_H
