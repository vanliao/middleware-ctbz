#ifndef SQLITE_DB_H
#define SQLITE_DB_H

#include <sqlite3.h>
#include <vector>
#include <map>
#include "db.h"

namespace db {

class SqliteDB : public DB
{
public:
    SqliteDB();
    virtual ~SqliteDB();
    bool open(const std::string &file = "");
    void close();
    bool execute(const std::string &sql);
    bool getSqlResult(std::vector< std::map<std::string, std::string> > &result);

private:
    sqlite3* db;
    int exeCols;
    int exeRows;
    char **exeResult;
    char *exeErrMsg = NULL;

};

}
#endif // SQLITE_DB_H
