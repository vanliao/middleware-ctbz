#include "cgi_db.h"
#include "tinylog.h"

namespace db {

CgiDB::CgiDB()
{
    return;
}

db::CgiDB::~CgiDB()
{
    return;
}

CgiDB *CgiDB::instance()
{
    static CgiDB inst;
    return &inst;
}

bool CgiDB::readConfig(inifile::IniFileHelper &cfg)
{
    int ret = cfg.GetStringValue("DataBase", "file", &dbFile);

    if (0 != ret)
    {
        log_error("read db config failed " << ret);
        return false;
    }

    db.open(dbFile);

    return true;
}

bool CgiDB::authentication(const std::string &user, DBAuthentification &auth)

{
    std::lock_guard<std::mutex> guard(lock);

    std::string sql = std::string("select username, password from web_user where username = ") +
                        '"' + user + '"' + ";";
    if (!db.execute(sql))
    {
        return false;
    }

    std::vector<std::map<std::string, std::string> > result;
    if (!db.getSqlResult(result))
    {
        return false;
    }

    auth.userName = result[0]["username"];
    auth.password = result[0]["password"];

    return true;
}

}
