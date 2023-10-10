QT -= gui

CONFIG += c++17 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
INCLUDEPATH += dev log msg network main service public db \
               thirdparty/inifile thirdparty/xml thirdparty/rapidjson d:/project/include/

SOURCES += \
    db/cgi_db.cpp \
    db/sqlite_db.cpp \
    dev/end_point.cpp \
    dev/lte_dev_demo.cpp \
    dev/nr_dev_demo.cpp \
    dev/ws_dev_demo.cpp \
    thirdparty/inifile/inifile.cpp \
    log/tinylog.cpp \
    log/tlog.c \
    main/main.cpp \
    main/main_client_demo.cpp \
    main/main_server_demo.cpp \
    main/main_ssl_server_demo.cpp \
    main/main_ws_client_demo.cpp \
    main/main_ws_server_demo.cpp \
    msg/msg.cpp \
    msg/server_demo_msg.cpp \
    network/epoll_communicator.cpp \
    network/epoll_server.cpp \
    network/socket.cpp \
    network/ssl_client.cpp \
    network/ssl_common.cpp \
    network/ssl_server.cpp \
    network/tcp_client.cpp \
    network/tcp_server.cpp \
    network/tcp_socket.cpp \
    network/udp_client.cpp \
    network/udp_server.cpp \
    network/udp_socket.cpp \
    network/websocket_client.cpp \
    network/websocket_server.cpp \
    public/api.cpp \
    service/client_demo.cpp \
    service/common_communicator.cpp \
    service/common_server.cpp \
    service/server_demo.cpp \
    thirdparty/xml/tinyxml2.cpp \
    service/ssl_server_demo.cpp \
    service/ws_client_demo.cpp \
    service/ws_server_demo.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    cgi.drawio \
    config/config.ini \
    config/nm.db \
    lib/libcrypto.a \
    lib/libfcgi.a \
    lib/libfcgi.a.pc \
    lib/libsqlite3.a \
    lib/libssl.a \
    lib/readme.txt \
    run.sh \
    test/.idea/.gitignore \
    test/.idea/inspectionProfiles/Project_Default.xml \
    test/.idea/inspectionProfiles/profiles_settings.xml \
    test/.idea/misc.xml \
    test/.idea/modules.xml \
    test/.idea/test.iml \
    test/.idea/workspace.xml \
    test/acs_data.py \
    test/lte_tcp.py \
    test/lte_udp.py \
    test/nr_tcp.py \
    test/nr_udp.py \
    test/test.py

HEADERS += \
    db/cgi_db.h \
    db/db.h \
    db/db_data.h \
    db/sqlite_db.h \
    dev/dev.h \
    dev/end_point.h \
    dev/lte_dev_demo.h \
    dev/nr_dev_demo.h \
    dev/ws_dev_demo.h \
    thirdparty/inifile/inifile.h \
    lib/fcgi/fastcgi.h \
    lib/fcgi/fcgi_config.h \
    lib/fcgi/fcgi_stdio.h \
    lib/fcgi/fcgiapp.h \
    lib/fcgi/fcgimisc.h \
    lib/fcgi/fcgio.h \
    lib/fcgi/fcgios.h \
    lib/openssl/aes.h \
    lib/openssl/asn1.h \
    lib/openssl/asn1_mac.h \
    lib/openssl/asn1t.h \
    lib/openssl/bio.h \
    lib/openssl/blowfish.h \
    lib/openssl/bn.h \
    lib/openssl/buffer.h \
    lib/openssl/camellia.h \
    lib/openssl/cast.h \
    lib/openssl/cmac.h \
    lib/openssl/cms.h \
    lib/openssl/comp.h \
    lib/openssl/conf.h \
    lib/openssl/conf_api.h \
    lib/openssl/crypto.h \
    lib/openssl/des.h \
    lib/openssl/des_old.h \
    lib/openssl/dh.h \
    lib/openssl/dsa.h \
    lib/openssl/dso.h \
    lib/openssl/dtls1.h \
    lib/openssl/e_os2.h \
    lib/openssl/ebcdic.h \
    lib/openssl/ec.h \
    lib/openssl/ecdh.h \
    lib/openssl/ecdsa.h \
    lib/openssl/engine.h \
    lib/openssl/err.h \
    lib/openssl/evp.h \
    lib/openssl/hmac.h \
    lib/openssl/idea.h \
    lib/openssl/krb5_asn.h \
    lib/openssl/kssl.h \
    lib/openssl/lhash.h \
    lib/openssl/md4.h \
    lib/openssl/md5.h \
    lib/openssl/mdc2.h \
    lib/openssl/modes.h \
    lib/openssl/obj_mac.h \
    lib/openssl/objects.h \
    lib/openssl/ocsp.h \
    lib/openssl/opensslconf.h \
    lib/openssl/opensslv.h \
    lib/openssl/ossl_typ.h \
    lib/openssl/pem.h \
    lib/openssl/pem2.h \
    lib/openssl/pkcs12.h \
    lib/openssl/pkcs7.h \
    lib/openssl/pqueue.h \
    lib/openssl/rand.h \
    lib/openssl/rc2.h \
    lib/openssl/rc4.h \
    lib/openssl/ripemd.h \
    lib/openssl/rsa.h \
    lib/openssl/safestack.h \
    lib/openssl/seed.h \
    lib/openssl/sha.h \
    lib/openssl/srp.h \
    lib/openssl/srtp.h \
    lib/openssl/ssl.h \
    lib/openssl/ssl2.h \
    lib/openssl/ssl23.h \
    lib/openssl/ssl3.h \
    lib/openssl/stack.h \
    lib/openssl/symhacks.h \
    lib/openssl/tls1.h \
    lib/openssl/ts.h \
    lib/openssl/txt_db.h \
    lib/openssl/ui.h \
    lib/openssl/ui_compat.h \
    lib/openssl/whrlpool.h \
    lib/openssl/x509.h \
    lib/openssl/x509_vfy.h \
    lib/openssl/x509v3.h \
    lib/sqlite3/sqlite3.h \
    lib/sqlite3/sqlite3ext.h \
    log/tinylog.h \
    log/tlog.h \
    main/main_client_demo.h \
    main/main_server_demo.h \
    main/main_ssl_server_demo.h \
    main/main_ws_client_demo.h \
    main/main_ws_server_demo.h \
    msg/msg.h \
    msg/server_demo_msg.h \
    network/epoll_communicator.h \
    network/epoll_server.h \
    network/socket.h \
    network/ssl_client.h \
    network/ssl_common.h \
    network/ssl_server.h \
    network/tcp_client.h \
    network/tcp_server.h \
    network/tcp_socket.h \
    network/udp_client.h \
    network/udp_server.h \
    network/udp_socket.h \
    network/websocket.h \
    network/websocket_client.h \
    network/websocket_server.h \
    public/api.h \
    thirdparty/rapidjson/allocators.h \
    thirdparty/rapidjson/cursorstreamwrapper.h \
    thirdparty/rapidjson/document.h \
    thirdparty/rapidjson/encodedstream.h \
    thirdparty/rapidjson/encodings.h \
    thirdparty/rapidjson/error/en.h \
    thirdparty/rapidjson/error/error.h \
    thirdparty/rapidjson/filereadstream.h \
    thirdparty/rapidjson/filewritestream.h \
    thirdparty/rapidjson/fwd.h \
    thirdparty/rapidjson/internal/biginteger.h \
    thirdparty/rapidjson/internal/diyfp.h \
    thirdparty/rapidjson/internal/dtoa.h \
    thirdparty/rapidjson/internal/ieee754.h \
    thirdparty/rapidjson/internal/itoa.h \
    thirdparty/rapidjson/internal/meta.h \
    thirdparty/rapidjson/internal/pow10.h \
    thirdparty/rapidjson/internal/regex.h \
    thirdparty/rapidjson/internal/stack.h \
    thirdparty/rapidjson/internal/strfunc.h \
    thirdparty/rapidjson/internal/strtod.h \
    thirdparty/rapidjson/internal/swap.h \
    thirdparty/rapidjson/istreamwrapper.h \
    thirdparty/rapidjson/memorybuffer.h \
    thirdparty/rapidjson/memorystream.h \
    thirdparty/rapidjson/msinttypes/inttypes.h \
    thirdparty/rapidjson/msinttypes/stdint.h \
    thirdparty/rapidjson/ostreamwrapper.h \
    thirdparty/rapidjson/pointer.h \
    thirdparty/rapidjson/prettywriter.h \
    thirdparty/rapidjson/rapidjson.h \
    thirdparty/rapidjson/reader.h \
    thirdparty/rapidjson/schema.h \
    thirdparty/rapidjson/stream.h \
    thirdparty/rapidjson/stringbuffer.h \
    thirdparty/rapidjson/writer.h \
    service/client_demo.h \
    service/common_communicator.h \
    service/common_server.h \
    service/server_demo.h \
    thirdparty/xml/tinyxml2.h \
    service/ssl_server_demo.h \
    service/ws_client_demo.h \
    service/ws_server_demo.h
