#include "lte_dev_demo.h"
#include "rapidjson.h"
#include "writer.h"
#include "stringbuffer.h"
#include "document.h"
#include "error/en.h"
#include "tinylog.h"
#include "api.h"

namespace dev {

LteDevDemo::LteDevDemo(): Dev()
{
    headerLen = 19;
    msgCallback.insert(std::pair<int, std::function<Dev::ProcResult(const rapidjson::Document &doc, std::shared_ptr<msg::Msg> &outMsg)> >
                    (msg::type::cgi::MSG_TEST,
                     std::bind(&LteDevDemo::procMsgDemo, this,
                               std::placeholders::_1,std::placeholders::_2)));
    /////
    evCallback.insert(std::pair<int, std::function<Dev::ProcResult(const std::shared_ptr<msg::Msg> &msg, std::shared_ptr<msg::Msg> &outMsg)> >
    (msg::type::cgi::MSG_TEST,
     std::bind(&LteDevDemo::procEventDemo, this,
               std::placeholders::_1,std::placeholders::_2)));
}

LteDevDemo::~LteDevDemo()
{

}

Dev::ProcResult LteDevDemo::procEvent(const std::shared_ptr<msg::Msg> &msg, std::shared_ptr<msg::Msg> &outMsg)
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

Dev::ProcResult LteDevDemo::recv(const std::string &data, std::shared_ptr<msg::Msg> &outMsg)
{
//    ProcResult pr = ProcResult::RECVMORE;
//    msgBuf += data;
//    if (msgBuf.length() > headerLen)
//    {
//        /* JSON消息 */
//        log_debug("lte dev proc json msg");

//        std::string header = msgBuf.substr(0, headerLen);
//        std::string::size_type pos = header.find("MSG_LENGTH");
//        if (std::string::npos == pos || 0 != pos )
//        {
//            log_error("lte dev recv invalid msg");
//            return ProcResult::RECVMORE;
//        }

//        /* "MSG_LENGTH:"的长度是11 */
//        unsigned int bodyLen = api::str2int(header.substr(11).c_str());
//        if (msgBuf.length() < headerLen + bodyLen)
//        {
//            return ProcResult::RECVMORE;
//        }

//        std::string body = msgBuf.substr(headerLen, bodyLen);
//        msgBuf.erase(0, headerLen + bodyLen);

//        rapidjson::Document doc;
//        if (doc.Parse(body.c_str()).HasParseError())
//        {
//            log_error("lte dev json error:" << GetParseError_En(doc.GetParseError()));
//            return ProcResult::RECVMORE;
//        }

//        pr = procDevMsg(doc, outMsg);
//    }

//    return pr;
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

Dev::ProcResult LteDevDemo::procDevMsg(rapidjson::Document &doc, std::shared_ptr<msg::Msg> &outMsg)
{
    ProcResult pr = ProcResult::NONE;
    auto it = msgCallback.find(doc["code"].GetInt());
    if (msgCallback.end() != it)
    {
        pr = it->second(doc, outMsg);
    }
    else
    {
        log_error("lte dev unsupport msg");
    }
    return pr;
}

Dev::ProcResult LteDevDemo::procMsgDemo(const rapidjson::Document &doc, std::shared_ptr<msg::Msg> &outMsg)
{
    log_debug("proc msg demo");
    outMsg = std::make_shared<msg::Msg>();
    outMsg->raw = "{\"code\":1,\"data\":\"xyzabcdefg\"}";
    return ProcResult::SENDTODEV;
}

Dev::ProcResult dev::LteDevDemo::procEventDemo(const std::shared_ptr<msg::Msg> &msg, std::shared_ptr<msg::Msg> &outMsg)
{
    log_debug("proc event demo");
    return ProcResult::NONE;
}

}
