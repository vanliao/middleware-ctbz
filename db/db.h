#include <iostream>

namespace db {
class DB
{
public:
    DB(){};
    virtual ~DB(){};
    virtual bool open(const std::string &param = "") = 0;
    virtual void close() = 0;
    virtual bool execute(const std::string &sql) = 0;
};
}
