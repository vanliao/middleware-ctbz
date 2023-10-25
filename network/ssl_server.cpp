#include "tinylog.h"
#include "ssl_server.h"
#include "ssl_client.h"
#include "api.h"

namespace network {

SSLServer::SSLServer(const std::string serverIP, const int serverPort) :
    TcpServer(serverIP, serverPort), verifyCA(false), caFile(""), certFile(""), privateKeyFile("")
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

void SSLServer::setCAFile(const bool verifyPeer,
                          const std::string &caFilePath,
                          const std::string &certFilePath,
                          const std::string &keyFilePath)
{
    verifyCA = verifyPeer;
    caFile = caFilePath;
    certFile = certFilePath;
    privateKeyFile = keyFilePath;
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

    if (!SSL_CTX_set_cipher_list(sslCtx, "ALL"))
    {
        SSL_CTX_free(sslCtx);
        sslCtx = NULL;
        log_error("ssl set cipher list failed");
        return false;
    }

    if (verifyCA)
    {
        if (!caFile.empty())
        {
            /* 设置信任根证书 */
            if (SSL_CTX_load_verify_locations(sslCtx, caFile.c_str(), NULL)<=0)
            {
                SSL_CTX_free(sslCtx);
                sslCtx = NULL;
                log_error("ssl load verify locations failed");
                return false;
            }
        }
        else
        {
            log_error("ssl ca file not set");
            return false;
        }

        /*设置对端证书验证*/
        SSL_CTX_set_verify(sslCtx, SSL_VERIFY_PEER, NULL);
//        SSL_CTX_set_verify(sslCtx, SSL_VERIFY_NONE,NULL);
//        SSL_CTX_set_options (sslCtx, SSL_OP_ALL | SSL_OP_NO_SSLv2 |SSL_OP_NO_SSLv3);
//        SSL_CTX_set_mode(sslCtx, SSL_MODE_AUTO_RETRY);
    }

    if (!TcpServer::listen())
    {
        SSL_CTX_free(sslCtx);
        sslCtx = NULL;
        return false;
    }

    return true;
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
        close(sock);
        log_error("ssl new failed");
        return false;
    }

    /*将连接的socket加入到ssl*/
    SSL_set_fd(sslHandle, sock);

    /*建立ssl连接（握手）*/
    int ret = SSL_accept(sslHandle);
    if(ret <= 0)
    {
        close(sock);
        SSL_free(sslHandle);
        log_error("ssl accept failed:" << SSL_get_error(sslHandle, ret));
        return false;
    }

//    if (verifyCA)
//    {
//        if (!verifyPeerCA(sslHandle))
//        {
//            close(sock);
//            SSL_free(sslHandle);
//            log_error("ssl verify ca failed");
//            return false;
//        }
//    }

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

bool SSLServer::verifyPeerCA(SSL *sslHandle)
{
    X509* pX509Cert = NULL;
    X509_NAME *pX509Subject = NULL;
    char szCommName[256]={0};
    char szSubject[1024]={0};
    char szIssuer[256]={0};

    /*获取验证对端证书的结果*/
    if(X509_V_OK != SSL_get_verify_result(sslHandle))
    {
        log_error("verify peer cert failed");
        return false;
    }

    /*获取对端证书*/
    pX509Cert = SSL_get_peer_certificate(sslHandle);
    if(NULL == pX509Cert)
    {
        log_error("get peer cert failed");
        return false;
    }

    /*获取证书使用者属性*/
    pX509Subject = X509_get_subject_name(pX509Cert);
    if( NULL == pX509Subject)
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
