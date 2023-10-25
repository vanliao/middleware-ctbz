#ifndef END_POINT_H
#define END_POINT_H

#include "dev.h"

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

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
