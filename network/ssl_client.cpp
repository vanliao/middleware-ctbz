#include "tinylog.h"
#include "ssl_client.h"

namespace network {

SSLClient::SSLClient(const std::string serverIP, const int serverPort) :
    TcpClient(serverIP, serverPort), sslCtx(NULL)
{

}

SSLClient::SSLClient(const int clientFd):
    TcpClient(clientFd), sslCtx(NULL)
{

}

network::SSLClient::~SSLClient()
{
    if (sslCtx)
    {
        /*释放SSL上下文环境变量函数*/
        SSL_CTX_free(sslCtx);
        sslCtx = NULL;
    }
}

bool SSLClient::SSLconnect()
{
    const SSL_METHOD  *pMethod = SSLv23_client_method();
    if(NULL == pMethod)
    {
        log_error("ssl client method failed");
        return false;
    }

    /*初始化SSL上下文环境变量函数*/
    sslCtx = SSL_CTX_new(pMethod);
    if(NULL == sslCtx)
    {
        log_error("ssl ctx new failed");
        return false;
    }

    /*基于pCtx产生一个新的ssl*/
    sslHandle = SSL_new(sslCtx);
    if(NULL  == sslHandle)
    {
        SSL_CTX_free(sslCtx);
        sslCtx = NULL;
        log_error("ssl new failed");
        return false;
    }
    /*将连接的socket加入到ssl*/
    SSL_set_fd(sslHandle, fd);

    /*ssl握手*/
    int ret = SSL_connect(sslHandle);
    if(ret < 0)
    {
        int sslErrCode = SSL_get_error(sslHandle, ret);
        if (SSL_ERROR_WANT_WRITE == sslErrCode ||
            SSL_ERROR_WANT_READ == sslErrCode ||
            SSL_ERROR_WANT_X509_LOOKUP == sslErrCode)
        {
            status = SSLCONNECTING;
        }
        else
        {
            SSL_free(sslHandle);
            sslHandle = NULL;
            SSL_CTX_free(sslCtx);
            sslCtx = NULL;
            log_error("ssl connect failed:" << sslErrCode);
            return false;
        }
    }
//    else
//    {
//        status = CONNECTED;
//    }

    return true;
}

bool SSLClient::recv(std::string &buf)
{
    char buffer[8192];
    while (1)
    {
        int ret = SSL_read(sslHandle, buffer, sizeof(buffer));
        if (0 > ret)
        {
            int sslErrCode = SSL_get_error(sslHandle, ret);
            if (SSL_ERROR_NONE == sslErrCode)
            {
                if (SSL_pending(sslHandle))
                {
                    log_debug("ssl continue to read data");
                    continue;
                }
                else
                {
                    log_debug("ssl no more data to read");
                    break;
                }
            }
            else if (SSL_ERROR_WANT_WRITE == sslErrCode ||
                     SSL_ERROR_WANT_READ == sslErrCode ||
                     SSL_ERROR_WANT_X509_LOOKUP == sslErrCode)
            {
                /* 等同错误码 EAGAIN */
                log_debug("ssl recv eagain:" << sslErrCode);
                break;
            }
            else
            {
                log_error("ssl read failed:" << sslErrCode);
                return false;
            }
        }
        else
        {
            buf.append(buffer, ret);
            if ((unsigned int)ret < sizeof(buffer))
            {
                /* 接收的数据长度小于缓冲区长度说明接收完了 */
                break;
            }
        }
    }

    return true;
}

bool SSLClient::send(std::string &buf)
{
    std::string::size_type len = buf.length();
    int ret = 0;
    while (1)
    {
        ret = SSL_write(sslHandle, buf.c_str(), len);
        if (0 > ret)
        {
            int sslErrCode = SSL_get_error(sslHandle, ret);
            if (SSL_ERROR_WANT_WRITE == sslErrCode ||
                SSL_ERROR_WANT_READ == sslErrCode ||
                SSL_ERROR_WANT_X509_LOOKUP == sslErrCode)
            {
                /* 等同错误码 EAGAIN */
                log_debug("ssl send eagain:" << sslErrCode);
                break;
            }
            else
            {
                log_error("ssl send failed:" << sslErrCode);
                return false;
            }
        }
        else
        {
            break;
        }
    }

    buf.erase(0, ret);

    if ((unsigned int)ret != len)
    {
        log_debug("ssl send incomplete");
        return false;
    }

    return true;
}

}
