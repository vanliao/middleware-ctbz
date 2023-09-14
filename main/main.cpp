#include <unistd.h>
#include <string>
#include <csignal>
#include "server_demo_msg.h"
#include "tinylog.h"
#include "oam_cgi_server_demo.h"
#include "oam_cgi_client_demo.h"
#include "rapidjson.h"
#include "writer.h"
#include "stringbuffer.h"
#include "document.h"
#include "error/en.h"
#include "cgi_db.h"
#include "api.h"

void signalHandler(int /*signum*/)
{
    OamCgiServerDemo::instance()->stop();
//    OamCgiClientDemo::instance()->stop();
    return;
}

void jsonTest(void)
{
    rapidjson::StringBuffer strBuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
    writer.StartObject();

    writer.Key("cmdType");
    writer.Int(1);

    writer.Key("code");
    writer.Int(2);

    writer.Key("msg");
    writer.String("");

    writer.Key("timestamp");
    writer.Int(time(NULL));

    writer.Key("data");
    writer.StartArray();

    writer.StartObject();
    writer.Key("cellNumber"); writer.Int(3);
    writer.Key("RFSwitch"); writer.Int(4);
    writer.Key("powerValue"); writer.Int(5);
    writer.EndObject();
    writer.EndArray();
    writer.EndObject();

    log_debug(strBuf.GetString());

    rapidjson::Document doc;
    if (doc.Parse(strBuf.GetString()).HasParseError())
    {
        log_error("json error:" << GetParseError_En(doc.GetParseError()));
        return;
    }

    if (doc.HasMember("timestamp") && doc["timestamp"].IsInt())
    {
        log_debug("timestamp is " << doc["timestamp"].GetInt());
    }

    if (!doc.HasMember("data") || !doc["data"].IsArray())
    {
        log_error("invalid data");
        return;
    }
    const rapidjson::Value &data = doc["data"];
    if (data.Size() == 0)
    {
        log_error("invalid data number");
        return;
    }
//    for (size_t i = 0; i < data.Size(); i++)
//    {
//        const rapidjson::Value &item = data[i];
//        if (!item.IsObject())
//        {
//            continue;
//        }
//        if (item.HasMember("RFSwitch") && item["RFSwitch"].IsInt())
//        {
//            log_debug("RFSwitch is " << item["RFSwitch"].GetInt());
//        }
//    }

    for (auto it = data.Begin(); it != data.End(); it++)
    {
        if (!it->IsObject())
        {
            continue;
        }
        if (it->HasMember("RFSwitch") && (*it)["RFSwitch"].IsInt())
        {
            log_debug("RFSwitch is " << (*it)["RFSwitch"].GetInt());
        }
    }
}

int main(int argc, char *argv[])
{
    signal(SIGUSR1, signalHandler);

    inifile::IniFileHelper &cfg = inifile::IniFileHelper::instance();
    bool err = cfg.Load("config/config.ini");
    if (err)
    {
        std::cout << "load config.ini failed:" << cfg.GetErrMsg() << std::endl;
        return -1;
    }

    bool ret = tinylog::TinyLog::instance()->readConfig(cfg);
    if (!ret)
    {
        std::cout << "tiny log read config file failed" << std::endl;
        return -1;
    }
    ret = tinylog::TinyLog::instance()->run();
    if (!ret)
    {
        std::cout << "tiny log start failed" << std::endl;
        return -1;
    }

    if (!db::CgiDB::instance()->readConfig(cfg))
    {
//        db::CgiDB::instance()->getUserInfo();
        std::cout << "cfg db read config failed" << std::endl;
        log_error("server demo read config failed");
        return -1;
    }

    if (OamCgiServerDemo::instance()->readConfig(cfg))
    {
        OamCgiServerDemo::instance()->start();
    }
    else
    {
        std::cout << "server demo read config failed" << std::endl;
        log_debug("server demo read config failed");
        return -1;
    }

//    if (OamCgiClientDemo::instance()->readConfig(cfg))
//    {
//        OamCgiClientDemo::instance()->start();
//    }
//    else
//    {
//        std::cout << "client demo read config failed" << std::endl;
//        log_debug("client demo read config failed");
//        return -1;
//    }

    jsonTest();

//    std::string before = "123<>?:?!@#$%^&*(\"{}4adsfds,./adsa56789";
    std::string before = "admin";
    std::string cry = api::encrypt_cbc(before, "www.kiwict.com#!", "kiwi_ran");
    std::string after = api::decrypt_cbc(cry.c_str(), "www.kiwict.com#!", "kiwi_ran");
    if (before == after)
    {
        log_info("cbc encode/decode OK " << cry.c_str() << before << " " << after);
    }
    else
    {
        log_info("cbc encode/decode ERROR " << before << " " << after);
    }

    std::string md5Pwd = api::getMd5Str(before.c_str(), before.length());

    log_debug("app start");

    std::shared_ptr<msg::Msg> msg = std::make_shared<msg::type::cgi::ServerDemoMsg>();
    msg::type::cgi::ServerDemoMsg *ev = dynamic_cast<msg::type::cgi::ServerDemoMsg *>(msg.get());
    ev->code = 1;
    ev->cmdType = 2;
    ev->result = 3;
    sleep(2);
    OamCgiServerDemo::instance()->addEvent(msg);
//    OamCgiClientDemo::instance()->addEvent(msg);

    OamCgiServerDemo::instance()->loop();
//    OamCgiClientDemo::instance()->loop();

    log_debug("app stop");
    return 0;
}
