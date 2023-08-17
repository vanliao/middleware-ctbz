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

class DBShortcutMenu
{
public:
    DBShortcutMenu() {};
    ~DBShortcutMenu() {};
    void json(rapidjson::Writer<rapidjson::StringBuffer> &writer)
    {
        writer.Key("id");
        writer.Int(id);
        writer.Key("menuName");
        writer.String(menuName.c_str());
        writer.Key("iconName");
        writer.String(iconName.c_str());
        writer.Key("command");
        writer.String(command.c_str());
        writer.Key("remark");
        writer.String(remark.c_str());
    }

    int id;
    std::string menuName;
    std::string iconName;
    std::string command;
    std::string remark;
};

class DBUser
{
public:
    DBUser() {};
    ~DBUser() {};
    void json(rapidjson::Writer<rapidjson::StringBuffer> &writer)
    {
        writer.Key("id");
        writer.Int(id);
        writer.Key("enabled");
        writer.Int(enabled);
        writer.Key("permission");
        writer.Int(permission);
        writer.Key("userName");
        writer.String(userName.c_str());
        writer.Key("fullName");
        writer.String(fullName.c_str());
        writer.Key("handPhone");
        writer.String(handPhone.c_str());
        writer.Key("userFace");
        writer.String(userFace.c_str());
        writer.Key("level");
        writer.String(level.c_str());
        writer.Key("telePhone");
        writer.String(telePhone.c_str());
        writer.Key("address");
        writer.String(address.c_str());
        writer.Key("datetime");
        writer.String(datetime.c_str());
        writer.Key("remark");
        writer.String(remark.c_str());
    }

    int id;
    int enabled;
    int permission;
    std::string userName;
    std::string password;
    std::string fullName;
    std::string handPhone;
    std::string userFace;
    std::string level;
    std::string telePhone;
    std::string address;
    std::string datetime;
    std::string remark;
};

class DBMenu
{
public:
    DBMenu() {};
    ~DBMenu() {};
    void json(rapidjson::Writer<rapidjson::StringBuffer> &writer)
    {
        writer.Key("id");
        writer.Int(id);
        writer.Key("keepAlive");
        writer.Int(keepAlive);
        writer.Key("requireAuth");
        writer.Int(requireAuth);
        writer.Key("parentId");
        writer.Int(parentId);
        writer.Key("enabled");
        writer.Int(enabled);
        writer.Key("leafNode");
        writer.Int(leafNode);
        writer.Key("url");
        writer.String(url.c_str());
        writer.Key("path");
        writer.String(path.c_str());
        writer.Key("component");
        writer.String(component.c_str());
        writer.Key("name");
        writer.String(name.c_str());
        writer.Key("iconCls");
        writer.String(iconCls.c_str());
    };

    void jsonChildNode(rapidjson::Writer<rapidjson::StringBuffer> &writer)
    {
        /* 逆序遍历构造json结构
         * 原因是childNode是逆序构造
         */
        writer.Key("children");
        writer.StartArray();
        for (auto child = childNodes.rbegin(); child != childNodes.rend(); ++child)
        {
            writer.StartObject();
            child->json(writer);
            child->jsonChildNode(writer);
            writer.EndObject();
        }
        writer.EndArray();
    }

    int id;
    std::string url;
    std::string path;
    std::string component;
    std::string name;
    std::string iconCls;
    int keepAlive;
    int requireAuth;
    int parentId;
    int enabled;
    int leafNode;
    std::list<DBMenu> childNodes;
};

}
#endif // DB_DATA_H
