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
openssl genrsa -des3 -out server.key 4096
服务器证书请求
openssl req -new -key server.key -out server.csr
服务器证书
openssl ca -in server.csr -out server.crt -cert demoCA/cacert.pem -keyfile demoCA/private/cakey.pem
====================================