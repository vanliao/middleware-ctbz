unbuntu 系统环境安装

apt-get install gcc-9-aarch64-linux-gnu g++-9-aarch64-linux-gnu
apt-get install sqlite3
apt-get install libsqlite3-dev
apt-get install libfcgi-dev
apt-get install nginx
apt-get install gdb
apt-get install libssl-dev
apt-get install unzip zip
====================================
安装Nginx + FastCGI

软件准备
(1)Linux机器
所有的程序都是部署在Linux操作系统上面的，因此建议要有一台专门的机器来安装Linux。我们使用的Linux是CentOS 7，
大家可根据项目组的实际情况来选择Linux的版本。

(2)Nginx源码包
到Nginx的官网(http://nginx.org/en/download.html)上去下载最新的源码包，我们使用的是nginx-1.11.9.tar.gz。

(3)spawn_fastcgi源码包
到网站https://github.com/lighttpd/spawn-fcgi/releases 上去下载源码包spawn-fcgi-1.6.4.tar.gz。


2.编译安装
1)编译安装Nginx(yum install nginx)
wget http://nginx.org/download/nginx-1.11.12.tar.gz
第一步，执行tar zxvf nginx-1.11.12.tar.gz 命令解压文件到当前目录下(具体的安装包名根据你下载的Nginx包进行确定)。
第二步，进入解压后的nginx-1.11.9目录执行./configure --prefix=/usr/local/nginx命令。
第三步，执行make命令。
第四步，执行make install命令。

2)编译安装spawn_fastcgi(yum install spawn-fcgi / apt-get install spawn-fcgi)
wget http://www.lighttpd.net/download/spawn-fcgi-1.6.4.tar.bz2
第一步，执行 tar -jxf spawn-fcgi-1.6.4.tar.bz2 命令解压文件到当前目录下。
第二步，进入spawn-fcgi-1.6.4目录执行./configure命令。
第三步，执行make命令。
第四部，将文件spawn-fcgi拷贝到/usr/local/nginx/sbin目录下(拷贝之前如果没有这个目录，请手动创建)。


3)编译安装FastCGI(yum install fcgi-devel / apt-get install libfcgi-dev)
https://github.com/Seaworth/resources/tree/master/fastCGI%E5%AE%89%E8%A3%85%E5%8C%85

#include "fcgi_stdio.h"
#include <stdlib.h>

修改 nginx 配置
在
centos: /usr/local/nginx/conf/nginx.conf
ubuntu: /etc/nginx/sites-available/default
配置文件中的http节点的server子节点下添加如下配置
location ~ /test.cgi$ {
    fastcgi_pass 127.0.0.1:8088;
    fastcgi_index index.cgi;
    fastcgi_param SCRIPT_FILENAME fcgi$fastcgi_script_name;

    include fastcgi_params;
}

编写FastCGI应用程序代码
int main(void)
{
    int count = 0;
    
    while (FCGI_Accept() >= 0)
    {
        printf("Content-type: text/html\r\n"
            "\r\n"
            "<title>Hello World</title>"
            "<h1>Hello World from FastCGI!</h1>"
            "Request number is: %d\n",
            ++count);
    }

    return 0;
}
g++ -o fastcgi_demo fastcgi_demo.cpp -lfcgi

启动 nginx
/usr/local/nginx/sbin/nginx
启动 fastcgi
spawn-fcgi -p 8088 -f ./fastcgi_demo
浏览器访问
http://192.168.216.131/test.cgi
====================================
交叉编译 sqlite
下载：https://www.sqlite.org/2023/sqlite-autoconf-3420000.tar.gz
./configure --host=aarch64-linux-gnu --prefix=/home/liaohui/sqliteInstall

====================================

交叉编译 fast CGI
修改config.sub脚本。为了让该脚本识别，对basic_machine加上aarch64识别
```
...

aarch64)
    # add by GTF.
    ;;
*-unknown)
...
...
```
./configure --host=aarch64-linux-gnu --prefix=/home/liaohui/fcgiInstall
make && make install

====================================

交叉编译 Spawn-fcgi
 ./configure --host=aarch64-linux-gnu 

====================================
编译 openssl

下载地址 选一个版本
https://github.com/openssl/openssl/releases/tag/OpenSSL_1_0_2k

unzip openssl-OpenSSL_1_0_2k.zip 

cd openssl-OpenSSL_1_0_2k/G

chmod -R 755 ./

export CC=aarch64-linux-gnu-gcc

export AR=aarch64-linux-gnu-ar

export CXX=aarch64-linux-gnu-g++

export RANLIB=aarch64-linux-gnu-ranlib

./config no-asm --prefix=/home/van/project/oamCGI/lib/openssl

config会生成Makefile文件，编辑makefile，去掉-m64，否则编译报错

make && make install
====================================
生成证书流程

mkdir demoCA
cd demoCA

mkdir -p certs crl newcerts private
touch index.txt
echo 01 >> serial

ca 秘钥
openssl genrsa -des3 -out private/cakey.pem 4096
ca 证书
openssl req -new -x509 -key private/cakey.pem  -out cacert.pem

cd ..

服务器秘钥
###需要输入密码:openssl genrsa -des3 -out server.key 4096
###不要输入密码:openssl genrsa -out server.key 4096
服务器证书请求
openssl req -new -key server.key -out server.csr
服务器证书
openssl ca -in server.csr -out server.crt -cert demoCA/cacert.pem -keyfile demoCA/private/cakey.pem

客户端秘钥
openssl genrsa -out client.key 4096
客户端证书请求
openssl req -new -key client.key -out client.csr
输入信息：C(国家)/ST(省份)/L(城市)/O(公司) 这4个内容必须和CA的一致
客户端证书
openssl ca -in client.csr -out client.crt -cert demoCA/cacert.pem -keyfile demoCA/private/cakey.pem

服务端和客户端证书必须使用同一个CA生成，否则验证失败

查看证书
openssl x509 -in cacert.pem -noout -text
openssl x509 -in server.crt -noout -text
openssl x509 -in client.crt -noout -text

Certificate:
    Data:
        Version: 3 (0x2)
        Serial Number: 4 (0x4)
        Signature Algorithm: sha256WithRSAEncryption
        Issuer: C = CN, ST = GD, L = SZ, O = SW, OU = SW, CN = SW, emailAddress = SW
        Validity
            Not Before: Oct 11 02:28:38 2023 GMT
            Not After : Oct 10 02:28:38 2024 GMT
        Subject: C = CN, ST = GD, O = SW, OU = 123123, CN = 123123, emailAddress = 123123
        Subject Public Key Info:
            Public Key Algorithm: rsaEncryption
                RSA Public-Key: (4096 bit)
                Modulus:
                    00:cc:73:32:0a:fd:1c:a6:e7:75:d9:2a:75:bd:ea:
                    cc:83:30:2f:0c:5e:be:36:e8:93:d5:54:21:19:c1:
                    38:7d:02:5d:40:70:24:87:41:93:9e:e2:07:26:81:
                    ad:82:b1:33:28:db:4a:f9:c3:80:44:6a:a5:1a:2e:
                    97:7b:22:a0:77:72:02:1d:ad:99:75:33:3a:4c:08:
                    4f:29:ad:31:9c:e7:d1:09:6d:56:78:f7:34:1c:ab:
                    bf:b9:a2:3f:d0:1c:a8:d6:c0:4e:9c:ab:33:0f:80:
                    5d:be:94:71:96:9f:84:8f:0a:70:c5:26:d0:da:28:
                    47:c5:fe:80:a6:86:02:73:5e:4c:89:5f:08:1f:9d:
                    35:31:78:83:57:89:ee:de:00:c1:a3:e4:fe:84:4e:
                    dc:25:c1:fa:a4:b6:e7:f5:89:b7:ee:d6:cb:16:7f:
                    5f:93:83:27:e9:f9:e8:dc:82:b0:c7:14:c0:ec:e7:
                    5a:5a:fb:53:e1:96:3c:91:a1:e3:50:eb:3c:c1:7a:
                    65:2c:32:36:8b:aa:9b:87:4c:ed:23:e0:00:0c:27:
                    34:57:a2:33:d3:be:b9:e8:89:62:ca:61:8f:29:38:
                    b9:2c:29:30:e7:76:7c:37:88:b8:92:9d:ad:16:4d:
                    ec:07:9a:24:4a:19:89:8e:a3:6e:90:a9:ab:65:36:
                    22:e4:cb:ff:84:65:ec:00:e9:aa:ac:db:5b:44:7b:
                    08:30:26:ee:ab:37:9a:24:2e:d6:7e:a8:2f:70:da:
                    9f:27:35:34:c8:46:a2:b4:75:2e:3a:b8:06:6f:ab:
                    1a:39:4d:cf:3d:d7:71:54:bb:4a:16:41:a5:62:d4:
                    43:d0:4f:16:22:f7:5a:1e:26:55:a0:02:f9:79:23:
                    0d:e9:05:d1:9a:5a:4f:ac:c1:04:09:83:7f:57:2b:
                    50:04:51:3c:14:26:f5:98:f1:3e:17:c1:6c:53:82:
                    14:ae:a8:20:20:46:cd:e6:ec:2d:20:49:5f:84:ee:
                    4d:3c:85:e0:6e:24:15:96:e1:ef:0d:ee:30:03:6d:
                    c3:87:20:60:cc:93:60:1c:f9:f3:78:35:9b:43:ac:
                    a6:01:7c:e3:1d:e5:b3:1c:d0:5e:09:29:e1:96:08:
                    e7:dc:2b:dc:e9:ed:7d:3a:1d:56:be:9b:a7:30:7d:
                    af:ff:3d:57:0f:f4:28:a8:27:fb:57:bf:4e:6a:e0:
                    a5:d2:d6:5c:46:c2:ec:8b:ba:e3:70:b6:0b:e8:2b:
                    4c:5f:6b:a9:0d:32:54:43:8e:16:d8:05:90:51:2a:
                    3e:77:a7:61:e7:bf:57:08:4d:f2:7b:81:73:a8:b4:
                    58:7e:74:0c:c2:f0:71:7e:06:b1:f4:95:89:4f:99:
                    dc:12:85
                Exponent: 65537 (0x10001)
        X509v3 extensions:
            X509v3 Basic Constraints:
                CA:FALSE
            Netscape Comment:
                OpenSSL Generated Certificate
            X509v3 Subject Key Identifier:
                63:BE:98:CE:EC:E0:3A:90:7B:DA:80:10:61:F6:31:85:B2:81:A4:57
            X509v3 Authority Key Identifier:
                keyid:C5:D9:0C:27:1D:9C:CE:F3:B1:EB:33:27:62:D8:1F:A2:F6:FA:CB:9B

    Signature Algorithm: sha256WithRSAEncryption
         cc:d1:34:45:b6:12:28:38:d9:c9:d4:12:3d:3e:a9:03:97:41:
         ab:cb:25:55:bd:49:e4:67:c4:eb:ba:74:9a:c9:bf:33:77:1d:
         e4:a0:e2:73:9c:6b:4e:ff:60:e0:7d:30:21:22:ba:3a:bb:f6:
         ac:d9:f5:05:0d:b2:df:e9:09:0e:a0:9a:4c:7c:89:97:ba:fe:
         95:53:48:13:69:f3:5e:88:35:2b:24:06:a9:86:02:62:b9:9b:
         69:2a:2f:63:29:e0:de:d6:e0:64:88:1f:5f:e4:ca:ed:4c:3a:
         d0:cb:ea:10:67:55:67:5c:71:70:c1:fc:98:1b:45:09:b7:f6:
         fd:e0:28:fd:10:c2:d8:4d:2d:de:dd:eb:f8:8d:49:20:44:78:
         70:e3:20:c7:2b:88:a8:22:16:43:90:31:49:ba:e8:60:f4:86:
         de:34:8b:5b:b2:1d:60:69:94:09:28:f0:c6:f4:5a:56:b8:43:
         aa:a7:f7:b7:f9:c9:8a:9b:46:49:c8:66:94:a4:4f:4e:28:02:
         16:72:1d:d5:c4:ad:11:40:c4:6c:92:ba:f8:fc:3c:92:3d:58:
         c3:99:e3:96:e0:1e:9b:08:dd:f4:07:57:27:23:d8:0c:8e:53:
         08:de:49:08:de:8e:4b:df:aa:45:b7:62:1b:b2:e7:5c:67:31:
         d8:d2:34:f1:4d:4e:9e:d6:65:a7:55:d7:fa:b8:ed:11:5c:5e:
         20:45:85:5c:b2:08:c6:cc:a0:03:bf:72:5f:8f:91:77:ce:68:
         b3:36:0b:3f:80:80:bc:90:5a:11:16:fd:78:fa:fa:04:47:0e:
         f3:44:b7:aa:12:e2:60:ee:84:32:4f:e4:1f:83:a8:43:68:87:
         52:75:3a:96:36:46:11:9c:0b:a8:c6:22:0f:a6:db:ee:24:2a:
         21:58:04:d8:60:5a:df:8c:be:17:47:67:49:5c:33:9a:c7:90:
         5b:21:2c:60:b5:08:07:ac:2e:7c:69:14:cd:4c:92:de:c8:1b:
         6f:d3:4d:5f:93:34:4a:2c:75:66:52:b6:b7:70:5b:94:7b:97:
         de:2c:34:57:48:b0:01:21:aa:0f:48:87:ef:12:b9:7f:82:25:
         49:cb:1c:26:22:7a:59:bc:41:8d:e9:52:96:37:25:3b:77:d0:
         c0:6a:b5:9d:b5:50:56:e9:68:3b:53:5b:58:62:d8:5c:1a:e7:
         60:d3:3e:06:26:a3:11:cd:48:82:30:93:33:76:6e:c2:0d:e3:
         7b:b0:d3:87:bc:8a:95:47:46:82:78:8c:04:fc:e9:de:4d:8c:
         2f:11:42:23:71:b7:a6:33:c7:2b:77:b7:59:1e:9f:90:ad:79:
         55:b2:01:24:e9:2c:41:c1

====================================
