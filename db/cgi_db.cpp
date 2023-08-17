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

bool CgiDB::getShortCutMenu(std::vector<DBShortcutMenu> &menu)
{
    std::lock_guard<std::mutex> guard(lock);

    std::string sql = std::string("select id,menu_name,icon_name,command,remark from web_shortcut_menu");
    if (!db.execute(sql))
    {
        return false;
    }

    std::vector<std::map<std::string, std::string> > result;
    if (!db.getSqlResult(result))
    {
        return false;
    }

    for (unsigned int i = 0; i < result.size(); i++)
    {
        DBShortcutMenu item;
        item.id = std::atoi(result[i]["id"].c_str());
        item.menuName = result[i]["menu_name"];
        item.iconName = result[i]["icon_name"];
        item.command = result[i]["command"];
        item.remark = result[i]["remark"];
        menu.push_back(item);
    }

    return true;
}

bool CgiDB::getUserInfo(std::vector<DBUser> &userInfo, const std::string userName)
{
    std::lock_guard<std::mutex> guard(lock);

    std::string sql = std::string("select id,username,fullName,handPhone,userFace,"
                                  "password,level,telePhone,address,datetime,enabled,permission,remark "
                                  "from web_user where username=\"" + userName + "\"");
    if (!db.execute(sql))
    {
        return false;
    }

    std::vector<std::map<std::string, std::string> > result;
    if (!db.getSqlResult(result))
    {
        return false;
    }

    for (unsigned int i = 0; i < result.size(); i++)
    {
        DBUser item;
        item.id = std::atoi(result[i]["id"].c_str());
        item.enabled = std::atoi(result[i]["enabled"].c_str());
        item.permission = std::atoi(result[i]["permission"].c_str());
        item.userName = result[i]["username"];
        item.fullName = result[i]["fullName"];
        item.handPhone = result[i]["handPhone"];
        item.userFace = result[i]["userFace"];
        item.password = result[i]["password"];
        item.level = result[i]["level"];
        item.telePhone = result[i]["telePhone"];
        item.address = result[i]["address"];
        item.datetime = result[i]["datetime"];
        item.remark = result[i]["remark"];
        userInfo.push_back(item);
    }

    return true;
}

bool CgiDB::getMenu(std::map<int, db::DBMenu> &menuInfo)
{
    std::lock_guard<std::mutex> guard(lock);

    std::string sql = std::string("select id,url,path,component,name,leafNode,"
                                  "iconCls,keepAlive,requireAuth,parentId,enabled "
                                  "from web_menu where id > 1");
    if (!db.execute(sql))
    {
        return false;
    }

    std::vector<std::map<std::string, std::string> > result;
    if (!db.getSqlResult(result))
    {
        return false;
    }

    for (unsigned int i = 0; i < result.size(); i++)
    {
        DBMenu item;
        item.id = std::atoi(result[i]["id"].c_str());
        item.keepAlive = std::atoi(result[i]["keepAlive"].c_str());
        item.requireAuth = std::atoi(result[i]["requireAuth"].c_str());
        item.parentId = std::atoi(result[i]["parentId"].c_str());
        item.enabled = std::atoi(result[i]["enabled"].c_str());
        item.leafNode = std::atoi(result[i]["leafNode"].c_str());
        item.url = result[i]["url"];
        item.path = result[i]["path"];
        item.component = result[i]["component"];
        item.name = result[i]["name"];
        item.iconCls = result[i]["iconCls"];
        menuInfo.insert(std::pair<int, DBMenu>(item.id, item));
    }

    /* 逆序构造树形结构
     * 现将第1层的叶子节点加入第2层的父节点中
     * 再讲第2层节点加入到第3层,以此类推
     * 正序遍历会导致数据遗漏
     */
    for (auto it = menuInfo.rbegin(); it != menuInfo.rend(); ++it)
    {
        if (1 != it->second.parentId)
        {
            db::DBMenu &parentNode = menuInfo[it->second.parentId];
            parentNode.childNodes.push_back(it->second);
            it->second.id = -1;
        }
    }

    /* 除了最顶层的节点，其他的都删除 */
    auto it = menuInfo.begin();
    while (it != menuInfo.end())
    {
        if (-1 == it->second.id)
        {
            menuInfo.erase(it++);
        }
        else
        {
            it++;
        }
    }

    return true;
}

}
