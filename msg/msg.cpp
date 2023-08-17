#include <map>
#include "msg.h"
#include "tinylog.h"
#include "rapidjson.h"
#include "writer.h"
#include "stringbuffer.h"
#include "document.h"
#include "error/en.h"

namespace msg {

static std::map<int, std::string> msgName{
    {msg::type::cgi::MSG_BEGIN,  ""},
    {msg::type::cgi::MSG_TEST,  "TEST"},
};

static std::map<int, std::string> srcName{
    {msg::src::SRC_CGI, "CGI"},
    {msg::src::SRC_WEB, "WEB"},
    {msg::src::SRC_LTE, "LTE"},
    {msg::src::SRC_NR, "NR"},
};

void Msg::show(std::stringstream &ss, bool output)
{
     ss << std::endl;
     auto it = msgName.find(type);
     if (msgName.end() == it)
     {
         ss << "msg type:unkown(" << type << ")" << std::endl;
     }
     else
     {
        ss << "msg type:" << it->second << std::endl;
     }

     it = srcName.find(src);
     if (srcName.end() == it)
     {
         ss << "src:invalid(" << src << ")" << std::endl;
     }
     else
     {
         ss << "src:" << it->second << std::endl;
     }

     ss << "time:" << tm << std::endl;
     ss << "msg len:" << raw.length();

     if (output)
     {
        log_info(ss.str());
     }

     return;
}

bool Msg::verify()
{
    auto it = srcName.find(src);
    if (srcName.end() == it)
    {
        log_error("invalid msg");
        return false;
    }

    bool ret = false;
    switch (src)
    {
    case src::SRC_CGI:
    {
        if (src::SRC_CGI * MSG_NUM < type && type < (src::SRC_CGI + 1) * MSG_NUM)
        {
            ret = true;
        }
        break;
    }
    case src::SRC_WEB:
    {
        if (src::SRC_WEB * MSG_NUM < type && type < (src::SRC_WEB + 1) * MSG_NUM)
        {
            ret = true;
        }
        break;
    }
    case src::SRC_LTE:
    {
        if (src::SRC_LTE * MSG_NUM < type && type < (src::SRC_LTE + 1) * MSG_NUM)
        {
            ret = true;
        }
        break;
    }
    case src::SRC_NR:
    {
        if (src::SRC_NR * MSG_NUM < type && type < (src::SRC_NR + 1) * MSG_NUM)
        {
            ret = true;
        }
        break;
    }
    default:
    {
    ret = false;
    break;
    }
    }

    return ret;
}

std::string Msg::json()
{
    return "";
}

std::string Msg::json(unsigned int page, unsigned int pageSize)
{
    return "";
}

}
