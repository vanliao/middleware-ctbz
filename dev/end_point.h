#ifndef END_POINT_H
#define END_POINT_H

#include "dev.h"

namespace dev {

class EndPoint
{
public:
    EndPoint();
    virtual ~EndPoint();

public:
    std::shared_ptr<Dev> dev;
    unsigned int connID;
};

}

#endif // END_POINT_H
