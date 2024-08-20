#ifndef DB_DATA_H
#define DB_DATA_H

#include <sstream>
#include <list>
#include "rapidjson.h"
#include "writer.h"
#include "stringbuffer.h"
#include "document.h"
#include "error/en.h"

namespace db {

class DBAuthentification
{
public:
    DBAuthentification() {};
    ~DBAuthentification() {};

    std::string userName;
    std::string password;
};

}
#endif // DB_DATA_H
