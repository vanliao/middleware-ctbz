#ifndef DEV_H
#define DEV_H

#include <memory>
#include "msg.h"

namespace dev {

class Dev
{
public:
    Dev() {};
    virtual ~Dev() {};

    enum ProcResult
    {
        NONE,
        SENDTODEV,
        SENDTOCGI,
        RECVMORE,
    };
    virtual ProcResult procEvent(const std::shared_ptr<msg::Msg> &msg, std::shared_ptr<msg::Msg> &outMsg) = 0;
    virtual ProcResult recv(const std::string &data, std::shared_ptr<msg::Msg> &outMsg) = 0;

protected:
    std::string msgBuf;
};

}
#endif // DEV_H
