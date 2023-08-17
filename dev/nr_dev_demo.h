#ifndef NR_DEV_DEMO_H
#define NR_DEV_DEMO_H

#include <map>
#include <functional>
#include "dev.h"
#include "document.h"

namespace dev {

class NrDevDemo: public Dev
{
public:
    NrDevDemo();
    virtual ~NrDevDemo();
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
#endif // NR_DEV_DEMO_H
