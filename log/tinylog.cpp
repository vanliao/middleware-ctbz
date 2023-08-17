#include "tinylog.h"

namespace tinylog {

TinyLog *TinyLog::instance()
{
    static TinyLog inst;
    return &inst;
}

bool TinyLog::readConfig(inifile::IniFileHelper &cfg)
{
    int ret = cfg.GetStringValue("Log", "log_file", &file);
    ret &= cfg.GetIntValue("Log", "log_level", &level);
    ret &= cfg.GetIntValue("Log", "log_num", &num);
    ret &= cfg.GetIntValue("Log", "log_size", &size);
    ret &= cfg.GetIntValue("Log", "on_screen", &onScreen);

    if (0 != ret)
    {
        return false;
    }

    return true;
}

bool TinyLog::run()
{
    int flag = TLOG_NOCOMPRESS;
    if (0 != onScreen)
    {
        flag |= TLOG_SCREEN;
    }

    int ret = tlog_init(file.c_str(), size, num, 0, flag);
    ret |= tlog_setlevel((tlog_level)level);

    if (0 != ret)
    {
        return  false;
    }

    return true;
}

tinylog::TinyLog::TinyLog()
{
    return;
}

TinyLog::~TinyLog()
{
    tlog_exit();
    return;
}

}
