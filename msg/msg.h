#ifndef MSG_H
#define MSG_H

#include <iostream>
#include <sstream>
#include <future>
#include <map>

#define MSG_NUM 1000

namespace msg {

namespace src {
    enum Src
    {
        SRC_CGI,		/* CGI */
        SRC_WEB,		/* WEB */
        SRC_LTE,		/* LTE */
        SRC_NR,			/* NR */
    };
}

namespace type {

namespace cgi {
    enum MsgType
    {
        MSG_BEGIN = src::SRC_CGI * MSG_NUM,
        MSG_TEST,
        MSG_END,
    };
}

namespace web {
    enum MsgType
    {
        MSG_BEGIN = src::SRC_WEB * MSG_NUM,
        MSG_END,
    };
}

namespace lte {
    enum MsgType
    {
        MSG_BEGIN = src::SRC_LTE * MSG_NUM,
        MSG_END,
    };
}


namespace nr {
    enum MsgType
    {
        MSG_BEGIN = src::SRC_NR*MSG_NUM,
        MSG_END
    };
}

}

class Msg
{
public:
    Msg(){raw = "";tm = time(NULL);};
    virtual ~Msg() {};
    virtual void show(std::stringstream &ss, bool output = true);
    bool verify(void);
    virtual std::string json(void);
    virtual std::string json(unsigned int page, unsigned int pageSize);

public:
    src::Src src;
    int type;
    std::string raw;
    unsigned long tm;
    std::promise< std::shared_ptr<msg::Msg> > prom;
};

}

#endif // MSG_H
