#include "tinylog.h"
#include "ssl_server.h"
#include "ssl_client.h"
#include "api.h"

namespace network {

SSLServer::SSLServer(const std::string serverIP, const int serverPort) :
    TcpServer(serverIP, serverPort), certFile(""), privateKeyFile(""), sslCtx(NULL)
{
    SSLCommon::initSSL();
}

SSLServer::~SSLServer(void)
{
    if (sslCtx)
    {
        /*释放SSL上下文环境变量函数*/
        SSL_CTX_free(sslCtx);
        sslCtx = NULL;
    }
}

void SSLServer::setCertificateFile(const std::string &filePath)
{
    certFile = filePath;
}

void SSLServer::setPrivateKeyFile(const std::string &filePath)
{
    privateKeyFile = filePath;
}

bool SSLServer::listen()
{
    if (certFile.empty() || privateKeyFile.empty())
    {
        log_error("ssl cert/key file not set");
        return false;
    }

    const SSL_METHOD  *pMethod = SSLv23_server_method();
    if(NULL == pMethod)
    {
        log_error("ssl server method failed");
        return false;
    }

    /*初始化SSL上下文环境变量函数*/
    sslCtx = SSL_CTX_new(pMethod);
    if(NULL == sslCtx)
    {
        log_error("ssl ctx new failed");
        return false;
    }

    /* 载入用户的数字证书， 此证书用来发送给客户端。 证书里包含有公钥 */
    if(SSL_CTX_use_certificate_file(sslCtx, certFile.c_str(), SSL_FILETYPE_PEM) <= 0)
    {
        SSL_CTX_free(sslCtx);
        sslCtx = NULL;
        log_error("ssl set cert file failed");
        return false;
    }
#if 0
    /*设置私钥的解锁密码*/
    SSL_CTX_set_default_passwd_cb_userdata(sslCtx, "123456789");
#endif
    /* 载入用户私钥 */
    if(SSL_CTX_use_PrivateKey_file(sslCtx, privateKeyFile.c_str(), SSL_FILETYPE_PEM) <= 0)
    {
        SSL_CTX_free(sslCtx);
        sslCtx = NULL;
        log_error("ssl set key file failed");
        return false;
    }

    /* 检查用户私钥是否正确 */
    if(SSL_CTX_check_private_key(sslCtx) <= 0)
    {
        SSL_CTX_free(sslCtx);
        sslCtx = NULL;
        log_error("ssl check key file failed");
        return false;
    }

    /*证书验证*/
    SSL_CTX_set_verify(sslCtx, SSL_VERIFY_NONE,NULL);
    SSL_CTX_set_options (sslCtx, SSL_OP_ALL | SSL_OP_NO_SSLv2 |SSL_OP_NO_SSLv3);
//    SSL_CTX_set_mode(sslCtx, SSL_MODE_AUTO_RETRY);

    return TcpServer::listen();
}

bool SSLServer::accept(unsigned int &connID)
{
    int sock;
    std::string ip;
    int port;
    if (!TcpSocket::accept(sock, ip, port))
    {
        return false;
    }

    /*基于pCtx产生一个新的ssl*/
    SSL *sslHandle = SSL_new(sslCtx);
    if(NULL  == sslHandle)
    {
        log_error("ssl new failed");
        return false;
    }

    /*将连接的socket加入到ssl*/
    SSL_set_fd(sslHandle, sock);

    /*建立ssl连接（握手）*/
    int ret = SSL_accept(sslHandle);
    if(ret <= 0)
    {
        SSL_free(sslHandle);
        log_error("ssl accept failed:" << SSL_get_error(sslHandle, ret));
        return false;
    }

    if (clients.end() != clients.find(sock))
    {
        SSL_free(sslHandle);
        log_error("socket[" << sock << "] has been used");
        return false;
    }

    std::shared_ptr<TcpClient> clt = std::make_shared<SSLClient>(sock);
    auto it = clients.insert(std::pair<unsigned int, std::shared_ptr<TcpClient> >(api::getClientID(), clt));
    if (it.second)
    {
        clt->sslHandle = sslHandle;
        connID = it.first->first;
        clt->connID = connID;
        clt->ip = ip;
        clt->port = port;
        log_debug("create connection success:" << connID);
    }
    else
    {
        SSL_free(sslHandle);
    }

    return it.second;
}

}
