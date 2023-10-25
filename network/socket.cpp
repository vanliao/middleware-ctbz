#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <string.h>
#include "socket.h"
#include "tinylog.h"

namespace network {

Socket::Socket(const int sockType): dev::EndPoint(), fd(-1), type(sockType), sslHandle(NULL), sslCtx(NULL)
{
    return;
}

Socket::Socket(const int sockFd, const int sockType):dev::EndPoint(), fd(sockFd), type(sockType), sslHandle(NULL)
{
    return;
}

Socket::~Socket()
{
    destroy();
    return;
}

bool Socket::create()
{
    if (fd > -1)
    {
        log_error("fd is on use");
        return false;
    }

    fd = socket(AF_INET, type, 0);
    if (-1 == fd)
    {
        log_error("create socket failed:" << strerror(errno));
        return false;
    }

    setNonBlock();

    log_debug("create fd " << fd);

    return true;
}

void Socket::destroy()
{
    if (sslHandle)
    {
        log_debug("close ssl handle");
//        SSL_shutdown(sslHandle);
        SSL_free(sslHandle);
        sslHandle = NULL;
    }

    if (sslCtx)
    {
        /*释放SSL上下文环境变量函数*/
        SSL_CTX_free(sslCtx);
        sslCtx = NULL;
    }

    if (0 <= fd)
    {
        log_debug("close fd " << fd);
        close(fd);
        fd = -1;
    }

    return;
}

bool Socket::setNonBlock()
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (0 > flags)
    {
        log_error("get socket flag failed:" << strerror(errno));
        return false;
    }

   flags = fcntl(fd,F_SETFL, flags|O_NONBLOCK);
   if (0 > flags)
   {
        log_error("set socket flag failed:" << strerror(errno));
        return false;
   }

   return true;
}

Socket::POLL_RESULT Socket::pollIn()
{
    return POLL_RESULT_SUCCESS;
}

Socket::POLL_RESULT Socket::pollOut()
{
    return POLL_RESULT_SUCCESS;
}

}
