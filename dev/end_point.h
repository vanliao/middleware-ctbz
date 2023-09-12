#ifndef END_POINT_H
#define END_POINT_H

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

#endif // END_POINT_H
