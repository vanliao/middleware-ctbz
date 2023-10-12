#include "tinylog.h"
#include "ssl_client.h"

namespace network {

SSLClient::SSLClient(const std::string &serverIP,
                     const int serverPort,
                     const bool verifyPeer,
                     const std::string &caFilePath,
                     const std::string &certFilePath,
                     const std::string &keyFilePath) :
    TcpClient(serverIP, serverPort),
    verifyCA(verifyPeer), caFile(caFilePath), certFile(certFilePath), privateKeyFile(keyFilePath)
{
    SSLCommon::initSSL();
}

SSLClient::SSLClient(const int clientFd):
    TcpClient(clientFd)
{

}

network::SSLClient::~SSLClient()
{
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
            log_error("ssl set key file failed");
            return false;
        }
    }
    if (!caFile.empty())
    {
        /*加载CA证书（对端证书需要用CA证书来验证）*/
        if(SSL_CTX_load_verify_locations(sslCtx, caFile.c_str(), NULL) !=1)
        {
            log_error("ssl set ca file failed");
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

//    if (verifyCA)
//    {
//        return verifyPeerCA();
//    }

    return true;
}

bool SSLClient::recv(std::string &buf)
{
    char buffer[8192];
    while (1)
    {
        int ret = SSL_read(sslHandle, buffer, sizeof(buffer));
        log_debug("ssl recv len:" << ret);
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
                if (verifyCA)
                {
                    verifyCA = false;
                    if (!verifyPeerCA())
                    {
                        buf = "";
                        return false;
                    }
                }
                break;
            }
            else
            {
                log_error("ssl read failed:" << sslErrCode);
                buf = "";
                return false;
            }
        }
        else if (0 == ret)
        {
            buf = "";
            return false;
        }
        else
        {
            if (verifyCA)
            {
                verifyCA = false;
                if (!verifyPeerCA())
                {
                    buf = "";
                    return false;
                }
            }
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
                return false;
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
            break;
        }
    }

    buf.erase(0, ret);

    if ((unsigned int)ret != len)
    {
        log_debug("ssl send incomplete:sendlen=" << ret << ", totallen=" << len);
        return false;
    }

    return true;
}

bool SSLClient::verifyPeerCA()
{
    X509* pX509Cert = NULL;
    X509_NAME *pX509Subject = NULL;
    char szCommName[256]={0};
    char szSubject[1024]={0};
    char szIssuer[256]={0};

    /*获取验证对端证书的结果*/
    if (X509_V_OK != SSL_get_verify_result(sslHandle))
    {
        log_error("verify peer cert failed");
        return false;
    }

    /*获取对端证书*/
    pX509Cert = SSL_get_peer_certificate(sslHandle);
    if (NULL == pX509Cert)
    {
        log_error("get peer cert failed");
        return false;
    }

    /*获取证书使用者属性*/
    pX509Subject = X509_get_subject_name(pX509Cert);
    if (NULL == pX509Subject)
    {
        X509_free(pX509Cert);
        log_error("get cert subject failed");
        return false;
    }

    X509_NAME_oneline(pX509Subject, szSubject, sizeof(szSubject) -1);
    X509_NAME_oneline(X509_get_issuer_name(pX509Cert), szIssuer, sizeof(szIssuer) -1);
    X509_NAME_get_text_by_NID(pX509Subject, NID_commonName, szCommName, sizeof(szCommName)-1);
    log_debug("cert info:" << szSubject << " " << szIssuer << " " << szCommName);

    X509_free(pX509Cert);
    return true;
}

}
