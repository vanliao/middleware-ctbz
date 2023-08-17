#ifndef TINYLOG_H
#define TINYLOG_H

#include <sstream>
#include <thread>
#include "inifile.h"
#include "tlog.h"

namespace tinylog {

class TinyLog
{
public:
    static TinyLog *instance(void);
    bool readConfig(inifile::IniFileHelper &cfg);
    bool run(void);

private:
    TinyLog();
    ~TinyLog();

private:
    int num;
    int level;
    int size;
    int onScreen;
    std::string file;
};

}

#define log_debug(logstr) \
{ \
    std::stringstream _ss; \
    _ss << "[" << std::this_thread::get_id() << "]" << logstr; \
    tlog(TLOG_DEBUG, "%s\n", _ss.str().c_str()); \
}

#define log_info(logstr) \
{ \
    std::stringstream _ss; \
    _ss << "[" << std::this_thread::get_id() << "]" << logstr; \
    tlog(TLOG_INFO, "%s\n", _ss.str().c_str()); \
}

#define log_notice(logstr) \
{ \
    std::stringstream _ss; \
    _ss << "[" << std::this_thread::get_id() << "]" << logstr; \
    tlog(TLOG_NOTICE, "%s\n", _ss.str().c_str()); \
}

#define log_warning(logstr) \
{ \
    std::stringstream _ss; \
    _ss << "[" << std::this_thread::get_id() << "]" << logstr; \
    tlog(TLOG_WARN, "%s\n", _ss.str().c_str()); \
}

#define log_error(logstr) \
{ \
    std::stringstream _ss; \
    _ss << "[" << std::this_thread::get_id() << "]" << logstr; \
    tlog(TLOG_ERROR, "%s\n", _ss.str().c_str()); \
}

#define log_fatal(logstr) \
{ \
    std::stringstream _ss; \
    _ss << "[" << std::this_thread::get_id() << "]" << logstr; \
    tlog(TLOG_FATAL, "%s\n", _ss.str().c_str()); \
}


#endif // TINYLOG_H
