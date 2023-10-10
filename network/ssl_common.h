#ifndef SSLCOMMON_H
#define SSLCOMMON_H

#include <mutex>
#include "openssl/ssl.h"
#include "openssl/crypto.h"
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/rsa.h"
#include "openssl/x509.h"

namespace network {

class SSLCommon
{
public:
    SSLCommon();
    virtual ~SSLCommon();
    static void initSSL(void);

private:
    static std::once_flag onceFlag;
};

}

#endif // SSLCOMMON_H
