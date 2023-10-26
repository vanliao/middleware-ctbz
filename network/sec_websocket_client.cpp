#include <functional>
#include "tinylog.h"
#include "sec_websocket_client.h"

namespace network {

SecWebsocketClient::SecWebsocketClient(const std::string &serverIP,
                                       const int serverPort,
                                       const bool verifyPeer,
                                       const std::string &caFilePath,
                                       const std::string &certFilePath,
                                       const std::string &keyFilePath) :
    WebsocketClient(serverIP, serverPort),
    verifyCA(verifyPeer), caFile(caFilePath), certFile(certFilePath), privateKeyFile(keyFilePath)
{
    SSLCommon::initSSL();
    pollOutAction = std::bind(&SecWebsocketClient::connectTCPAction, this);
    pollInAction = std::bind(&SecWebsocketClient::wsHandShakeAction1, this);
}

SecWebsocketClient::SecWebsocketClient(const int clientFd):
    WebsocketClient(clientFd)
{
    pollOutAction = std::bind(&SecWebsocketClient::sendAction, this);
    pollInAction = std::bind(&SecWebsocketClient::wsHandShakeAction2, this);
}

SecWebsocketClient::~SecWebsocketClient()
{
}

Socket::POLL_RESULT SecWebsocketClient::pollIn()
{
    return pollInAction();
}

Socket::POLL_RESULT SecWebsocketClient::pollOut()
{
    return pollOutAction();
}

bool SecWebsocketClient::SSLconnect()
{
    const SSL_METHOD  *pMethod = SSLv23_client_method();
    if (unlikely(NULL == pMethod))
    {
        log_error("wss client method failed");
        return false;
    }

    /* 初始化SSL上下文环境变量函数 */
    sslCtx = SSL_CTX_new(pMethod);
    if (unlikely(NULL == sslCtx))
    {
        log_error("wss ctx new failed");
        return false;
    }

    if (!certFile.empty())
    {
        /* 载入用户的数字证书， 此证书用来发送给客户端。 证书里包含有公钥 */
        if(SSL_CTX_use_certificate_file(sslCtx, certFile.c_str(), SSL_FILETYPE_PEM) <= 0)
        {
            log_error("ssl set cert file failed");
            return false;
        }
    }
    if (!privateKeyFile.empty())
    {
        /* 载入用户私钥 */
        if(SSL_CTX_use_PrivateKey_file(sslCtx, privateKeyFile.c_str(), SSL_FILETYPE_PEM) <= 0)
        {
            log_error("wss set key file failed");
            return false;
        }
    }
    if (!caFile.empty())
    {
        /* 加载CA证书（对端证书需要用CA证书来验证）*/
        if(SSL_CTX_load_verify_locations(sslCtx, caFile.c_str(), NULL) !=1)
        {
            log_error("wss set ca file failed");
            return false;
        }
    }

    if (verifyCA)
    {
        /*设置对端证书验证*/
        SSL_CTX_set_verify(sslCtx, SSL_VERIFY_PEER, NULL);
    }

    /*基于sslCtx产生一个新的ssl*/
    sslHandle = SSL_new(sslCtx);
    if (unlikely(NULL  == sslHandle))
    {
        SSL_CTX_free(sslCtx);
        sslCtx = NULL;
        log_error("wss new failed");
        return false;
    }
    /*将连接的socket加入到ssl*/
    SSL_set_fd(sslHandle, fd);

    /*ssl握手*/
    int ret = SSL_connect(sslHandle);
    if(likely(ret < 0))
    {
        int sslErrCode = SSL_get_error(sslHandle, ret);
        if (likely(SSL_ERROR_WANT_WRITE == sslErrCode ||
            SSL_ERROR_WANT_READ == sslErrCode ||
            SSL_ERROR_WANT_X509_LOOKUP == sslErrCode))
        {
            status = SSLCONNECTING;
        }
        else
        {
            SSL_free(sslHandle);
            sslHandle = NULL;
            SSL_CTX_free(sslCtx);
            sslCtx = NULL;
            log_error("wss connect failed:" << sslErrCode);
            return false;
        }
    }
//    else
//    {
//        status = CONNECTED;
//    }

//    if (verifyCA)
//    {
//        return verifyPeerCA();
//    }

    return true;
}

bool SecWebsocketClient::recv(std::string &buf)
{
    char buffer[8192];
    while (1)
    {
        int ret = SSL_read(sslHandle, buffer, sizeof(buffer));
        log_debug("wss recv len:" << ret);
        if (unlikely(ret <= 0))
        {
            if (0 == ret)
            {
                buf = "";
                return false;
            }

            int sslErrCode = SSL_get_error(sslHandle, ret);
            if (SSL_ERROR_NONE == sslErrCode)
            {
                if (SSL_pending(sslHandle))
                {
                    log_debug("wss continue to read data");
                    continue;
                }
                else
                {
                    log_debug("wss no more data to read");
                    break;
                }
            }
            else if (SSL_ERROR_WANT_WRITE == sslErrCode ||
                     SSL_ERROR_WANT_READ == sslErrCode ||
                     SSL_ERROR_WANT_X509_LOOKUP == sslErrCode)
            {
                /* 等同错误码 EAGAIN */
                log_debug("wss recv eagain:" << sslErrCode);
//                if (verifyCA)
//                {
//                    verifyCA = false;
//                    if (!verifyPeerCA())
//                    {
//                        buf = "";
//                        return false;
//                    }
//                }
                break;
            }
            else
            {
                log_error("wss read failed:" << sslErrCode);
                buf = "";
                return false;
            }
        }
        else
        {
//            if (verifyCA)
//            {
//                verifyCA = false;
//                if (!verifyPeerCA())
//                {
//                    buf = "";
//                    return false;
//                }
//            }
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

bool SecWebsocketClient::send(std::string &buf)
{
    std::string::size_type len = buf.length();
    int ret = 0;
    while (1)
    {
        ret = SSL_write(sslHandle, buf.c_str(), len);
        if (unlikely(ret < 0))
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
            else if (SSL_ERROR_ZERO_RETURN == sslErrCode)
            {
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
            buf.erase(0, ret);
            break;
        }
    }

//    if ((unsigned int)ret != len)
//    {
//        log_debug("ssl send incomplete:sendlen=" << ret << ", totallen=" << len);
//        return false;
//    }

    return true;
}

Socket::POLL_RESULT SecWebsocketClient::connectTCPAction()
{
    if (unlikely(CONNECTING != status))
    {
        log_error("invaild tcp status " << status);
        /* 状态错误 关闭连接 */
        return POLL_RESULT_CLOSE;
    }

    if (likely(SSLconnect()))
    {
        /* 之后收到写事件 执行WS连接流程 */
        pollOutAction = std::bind(&SecWebsocketClient::connectWSAction, this);

        /* SSL连接成功 需要继续监听写事件 进行 WS 层面连接 */
        return POLL_RESULT_SEND;
    }

    /* SSL连接失败 关闭连接 */
    return POLL_RESULT_CLOSE;
}

Socket::POLL_RESULT SecWebsocketClient::connectWSAction()
{
    if (unlikely(SSLCONNECTING != status))
    {
        log_error("invaild tcp status " << status);
        /* 状态错误 关闭连接 */
        return POLL_RESULT_CLOSE;
    }

    if (likely(connectWS()))
    {
        /* 之后收到写事件 执行数据发送流程 */
        pollOutAction = std::bind(&SecWebsocketClient::sendAction, this);
        return POLL_RESULT_READ;
    }

    /* WS连接失败 对方可能没完成 SSL ACCEPT 继续发送 */
    return POLL_RESULT_SEND;
}

Socket::POLL_RESULT SecWebsocketClient::sendAction()
{
    while (!sendBuf.empty())
    {
        std::string &s = sendBuf.front();
        if (likely(SecWebsocketClient::send(s)))
        {
            if (!s.empty())
            {
                /* 本次没发送完 之后继续发送 */
                return POLL_RESULT_SEND;
            }

            sendBuf.pop_front();
        }
        else
        {
            /* 发送时遇到错误 关闭连接 */
            return POLL_RESULT_CLOSE;
        }
    }

    if (likely(sendBuf.empty()))
    {
        if (WebSocket::CLOSE_COMPLETE == closeStatus)
        {
            log_debug("send ws close msg and close ws");
            /* 双方协商关闭完成 断开连接 */
            return POLL_RESULT_CLOSE;
        }

        /* 发送完成 */
        return POLL_RESULT_SUCCESS;
    }

    /* 还有数据 之后继续发送 */
    return POLL_RESULT_SEND;
}

Socket::POLL_RESULT SecWebsocketClient::wsHandShakeAction1()
{
    bool ret = SecWebsocketClient::recv(recvBuf);
    if (unlikely(!ret))
    {
        closeStatus = WebSocket::CLOSE_FORCE;
        return POLL_RESULT_CLOSE;
    }

    if (unlikely(std::string::npos == recvBuf.rfind("\r\n\r\n")))
    {
        /* HTTP 头以 \r\n\r\n 结束
         * 没找到需要等待继续接收
         */
        return POLL_RESULT_READ;
    }

    ret = handleHttpReply(recvBuf);
    if (likely(ret))
    {
        pollInAction = std::bind(&SecWebsocketClient::recvAction, this);
        status = CONNECTED;
        return POLL_RESULT_HANDSHAKE;
    }

    recvBuf = "";
    closeStatus = WebSocket::CLOSE_FORCE;
    /* 握手失败 断开连接 */
    return POLL_RESULT_CLOSE;
}

Socket::POLL_RESULT SecWebsocketClient::wsHandShakeAction2()
{
    bool ret = SecWebsocketClient::recv(recvBuf);
    if (unlikely(!ret))
    {
        return POLL_RESULT_CLOSE;
    }

    if (unlikely(std::string::npos == recvBuf.rfind("\r\n\r\n")))
    {
        /* HTTP 头以 \r\n\r\n 结束
         * 没找到需要等待继续接收
         */
        return POLL_RESULT_READ;
    }

    ret = handleHttpRequest(recvBuf);
    if (likely(ret))
    {
        pollInAction = std::bind(&SecWebsocketClient::recvAction, this);
        recvBuf = "";
        /* 握手完成 继续接收数据 */
        return POLL_RESULT_HANDSHAKE;
    }

    return POLL_RESULT_CLOSE;
}

Socket::POLL_RESULT SecWebsocketClient::recvAction()
{
    bool ret = SecWebsocketClient::recv(recvBuf);
    if (unlikely(!ret))
    {
        closeStatus = WebSocket::CLOSE_FORCE;
        return POLL_RESULT_CLOSE;
    }

//    if (unlikely(recvBuf.empty()))
//    {
//        closeStatus = WebSocket::CLOSE_FORCE;
//        return POLL_RESULT_CLOSE;
//    }

    return handleWSMsg(recvBuf);
}

}
