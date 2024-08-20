#ifndef CGI_DB_H
#define CGI_DB_H

#include <mutex>
#include <map>
#include "inifile.h"
#include "sqlite_db.h"
#include "db_data.h"

namespace db {

class CgiDB
{
public:
    static CgiDB *instance(void);
    bool readConfig(inifile::IniFileHelper &cfg);
    bool authentication(const std::string &user, DBAuthentification &auth);

private:
    CgiDB();
    virtual ~CgiDB();

private:
    SqliteDB db;
    std::string dbFile;
    std::mutex lock;
};

}
#endif // CGI_DB_H
