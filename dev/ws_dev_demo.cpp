#include "tinylog.h"
#include "ws_dev_demo.h"
#include "rapidjson.h"
#include "writer.h"
#include "stringbuffer.h"
#include "document.h"
#include "error/en.h"

namespace dev {

WSDevDemo::WSDevDemo()
{
    isLogin = true;
    msgCallback.insert(std::pair<std::string, std::function<Dev::ProcResult(const rapidjson::Document &doc, std::shared_ptr<msg::Msg> &outMsg)> >
                    ("login", std::bind(&WSDevDemo::procMsgLogin, this, std::placeholders::_1,std::placeholders::_2)));
    msgCallback.insert(std::pair<std::string, std::function<Dev::ProcResult(const rapidjson::Document &doc, std::shared_ptr<msg::Msg> &outMsg)> >
                    ("echo", std::bind(&WSDevDemo::procMsgEcho, this, std::placeholders::_1,std::placeholders::_2)));
    /////
    evCallback.insert(std::pair<int, std::function<Dev::ProcResult(const std::shared_ptr<msg::Msg> &msg, std::shared_ptr<msg::Msg> &outMsg)> >
    (msg::type::cgi::MSG_TEST, std::bind(&WSDevDemo::procEventDemo, this, std::placeholders::_1,std::placeholders::_2)));
}

WSDevDemo::~WSDevDemo()
{

}

Dev::ProcResult WSDevDemo::procEvent(const std::shared_ptr<msg::Msg> &msg, std::shared_ptr<msg::Msg> &outMsg)
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

Dev::ProcResult WSDevDemo::recv(const std::string &data, std::shared_ptr<msg::Msg> &outMsg)
{
    ProcResult pr = ProcResult::RECVMORE;
    msgBuf += data;

    /* JSON消息 */
    log_debug("ws dev proc json msg");

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
        log_error("ws dev json error:" << GetParseError_En(doc.GetParseError()));
        return ProcResult::RECVMORE;
    }

    pr = procDevMsg(doc, outMsg);

    return pr;
}

Dev::ProcResult WSDevDemo::procDevMsg(rapidjson::Document &doc, std::shared_ptr<msg::Msg> &outMsg)
{
    ProcResult pr = ProcResult::NONE;
    auto it = msgCallback.find(doc["msgType"].GetString());
    if (msgCallback.end() != it)
    {
        pr = it->second(doc, outMsg);
    }
    else
    {
        log_error("ws dev unsupport msg");
    }
    return pr;
}

Dev::ProcResult WSDevDemo::procMsgLogin(const rapidjson::Document &doc, std::shared_ptr<msg::Msg> &outMsg)
{
    log_debug("proc ws msg login");

    ProcResult pr = ProcResult::SENDTODEV;
    if (!doc.HasMember("password") || !doc["password"].IsString())
    {
        log_error("invalid json member:password");
        return pr;
    }

    std::string pwd = doc["password"].GetString();
    if ("123456" == pwd)
    {
        isLogin = true;
    }

    rapidjson::StringBuffer strBuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
    writer.StartObject();
    writer.Key("msgType");
    writer.String("loginRsp");
    writer.Key("status");
    writer.String("success");
    writer.EndObject();

    outMsg = std::make_shared<msg::Msg>();
    outMsg->raw = strBuf.GetString();

    return ProcResult::SENDTODEV;
}

Dev::ProcResult WSDevDemo::procMsgEcho(const rapidjson::Document &doc, std::shared_ptr<msg::Msg> &outMsg)
{
    log_debug("proc ws msg echo");

    ProcResult pr = ProcResult::SENDTODEV;
    if (!doc.HasMember("data") || !doc["data"].IsString())
    {
        log_error("invalid json member:data");
        return pr;
    }

    std::string data = doc["data"].GetString();
    if (isLogin)
    {
        rapidjson::StringBuffer strBuf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
        writer.StartObject();
        writer.Key("msgType");
        writer.String("echoRsp");
        writer.Key("status");
        writer.String(data.c_str());
        writer.EndObject();

        outMsg = std::make_shared<msg::Msg>();
        outMsg->raw = strBuf.GetString();
    }
    else
    {
        rapidjson::StringBuffer strBuf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
        writer.StartObject();
        writer.Key("msgType");
        writer.String("error");
        writer.Key("desc");
        writer.String("please login first");
        writer.EndObject();

        outMsg = std::make_shared<msg::Msg>();
        outMsg->raw = strBuf.GetString();
    }

    return ProcResult::SENDTODEV;
}

Dev::ProcResult WSDevDemo::procEventDemo(const std::shared_ptr<msg::Msg> &msg, std::shared_ptr<msg::Msg> &outMsg)
{
    log_debug("proc ws event demo");
    return ProcResult::NONE;
}

}
