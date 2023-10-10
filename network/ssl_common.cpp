#include "ssl_common.h"

namespace network {

std::once_flag SSLCommon::onceFlag;

SSLCommon::SSLCommon()
{
    return;
}

SSLCommon::~SSLCommon()
{
    return;
}

void SSLCommon::initSSL()
{
    std::call_once(onceFlag, []()
    {
        /*SSL库初始化（一个进程只初始化一次）*/
        SSL_library_init();
        /*载入所有ssl错误消息*/
        SSL_load_error_strings();
        /*载入所有ssl算法*/
        OpenSSL_add_all_algorithms();
    });
}

}
