#include "nr_dev_demo.h"
#include "rapidjson.h"
#include "writer.h"
#include "stringbuffer.h"
#include "document.h"
#include "error/en.h"
#include "tinylog.h"
#include "api.h"

namespace dev {

NrDevDemo::NrDevDemo(): Dev()
{
    headerLen = 19;
    msgCallback.insert(std::pair<int, std::function<Dev::ProcResult(const rapidjson::Document &doc, std::shared_ptr<msg::Msg> &outMsg)> >
                    (msg::type::cgi::MSG_TEST,
                     std::bind(&NrDevDemo::procMsgDemo, this,
                               std::placeholders::_1,std::placeholders::_2)));
    /////
    evCallback.insert(std::pair<int, std::function<Dev::ProcResult(const std::shared_ptr<msg::Msg> &msg, std::shared_ptr<msg::Msg> &outMsg)> >
    (msg::type::cgi::MSG_TEST,
     std::bind(&NrDevDemo::procEventDemo, this,
               std::placeholders::_1,std::placeholders::_2)));
}

NrDevDemo::~NrDevDemo()
{

}

Dev::ProcResult NrDevDemo::procEvent(const std::shared_ptr<msg::Msg> &msg, std::shared_ptr<msg::Msg> &outMsg)
{
    ProcResult ret = ProcResult::NONE;
    auto it = evCallback.find(msg->type);
    if (evCallback.end() != it)
    {
        ret = it->second(msg, outMsg);
    }
    else
    {
        log_error("unsupported event type");
    }

    return ret;
}

Dev::ProcResult NrDevDemo::recv(const std::string &data, std::shared_ptr<msg::Msg> &outMsg)
{
    ProcResult pr = ProcResult::RECVMORE;
    msgBuf += data;
    if (msgBuf.length() > headerLen)
    {
        /* JSON消息 */
        log_debug("nr dev proc json msg");

        std::string body = "";
        int len = 0;
        int open = 0;
        for (const auto &it : msgBuf)
        {
            if (it == '[' || it == '{')
            {
                ++open;
            }
            if (it == ']' || it == '}')
            {
                --open;
            }

            ++len;
            if (0 == open)
            {
                body = msgBuf.substr(0, len);
                msgBuf.erase(0, len);
                break;
            }
        }

        if ("" == body)
        {
            return ProcResult::RECVMORE;
        }

        rapidjson::Document doc;
        if (doc.Parse(body.c_str()).HasParseError())
        {
            log_error("GZSJ DEV json error:" << GetParseError_En(doc.GetParseError()));
            return ProcResult::RECVMORE;
        }

        pr = procDevMsg(doc, outMsg);
    }

    return pr;
}

Dev::ProcResult NrDevDemo::procDevMsg(rapidjson::Document &doc, std::shared_ptr<msg::Msg> &outMsg)
{
    ProcResult pr = ProcResult::NONE;
    auto it = msgCallback.find(doc["code"].GetInt());
    if (msgCallback.end() != it)
    {
        pr = it->second(doc, outMsg);
    }
    else
    {
        log_error("nr dev unsupport msg");
    }
    return pr;
}

Dev::ProcResult NrDevDemo::procMsgDemo(const rapidjson::Document &doc, std::shared_ptr<msg::Msg> &outMsg)
{
    log_debug("proc msg demo");
    outMsg = std::make_shared<msg::Msg>();
    outMsg->raw = "{\"code\":1,\"data\":\"opqrstxyz\"}";
    return ProcResult::SENDTODEV;
}

Dev::ProcResult NrDevDemo::procEventDemo(const std::shared_ptr<msg::Msg> &msg, std::shared_ptr<msg::Msg> &outMsg)
{
    log_debug("proc event demo");
    return ProcResult::NONE;
}

}
