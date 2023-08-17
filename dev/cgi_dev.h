#ifndef CGI_DEV_H
#define CGI_DEV_H

#include "dev.h"

namespace dev {

class EndPoint
{
public:
    EndPoint();
    virtual ~EndPoint();
    virtual bool send(const std::string &buf) = 0;

public:
    std::shared_ptr<Dev> dev;
};

}

#endif // CGI_DEV_H
