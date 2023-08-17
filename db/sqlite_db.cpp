#include "sqlite_db.h"
#include "tinylog.h"

namespace db {

SqliteDB::SqliteDB() : db(NULL), exeCols(0), exeRows(0), exeResult(NULL), exeErrMsg(NULL)
{
    return;
}

SqliteDB::~SqliteDB()
{
    close();
    return;
}

bool SqliteDB::open(const std::string &file)
{
    if (NULL != db)
    {
        log_error("sqlite had opened");
        return false;
    }

    int ret = sqlite3_open(file.c_str(), &db);
    if (SQLITE_OK != ret)
    {
        log_error("open " << file << " failed");
        return false;
    }

    return true;
}

void SqliteDB::close()
{
    if (NULL != exeErrMsg)
    {
        sqlite3_free(exeErrMsg);
        exeErrMsg = NULL;
    }

    if (NULL != exeResult)
    {
        sqlite3_free_table(exeResult);
        exeResult = NULL;
    }

    if (NULL == db)
    {
        return;
    }

    int ret = sqlite3_close(db);
    if (SQLITE_OK == ret)
    {
        db = NULL;
    }
    else
    {
        log_error("sqlite db close failed");
    }

    return;
}

bool SqliteDB::execute(const std::string &sql)
{
    if (NULL != exeErrMsg)
    {
        sqlite3_free(exeErrMsg);
        exeErrMsg = NULL;
    }

    if (NULL != exeResult)
    {
        sqlite3_free_table(exeResult);
        exeResult = NULL;
    }

    int ret = sqlite3_get_table(db, sql.c_str(), &exeResult, &exeRows, &exeCols, &exeErrMsg);
    if (SQLITE_OK != ret)
    {
        log_error("db execute failed:" << exeErrMsg);
        return false;
    }

    return true;
}

bool SqliteDB::getSqlResult(std::vector<std::map<std::string, std::string> > &result)
{
    if (0 >= exeRows || 0 >= exeCols || NULL == exeResult)
    {
        log_error("get sql result failed");
        return false;
    }

    int index = exeCols;
    for(int i = 0; i < exeRows; i ++)
    {
        std::map<std::string, std::string> row;
        for(int j = 0; j < exeCols; j ++)
        {
            row[exeResult[j]] = NULL==exeResult[index]?"":exeResult[index];
            index ++;
        }
        result.push_back(row);
    }

    return true;
}

}
