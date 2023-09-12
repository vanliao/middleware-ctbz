QT -= gui

CONFIG += c++17 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    db/cgi_db.cpp \
    db/sqlite_db.cpp \
    dev/end_point.cpp \
    dev/lte_dev_demo.cpp \
    dev/nr_dev_demo.cpp \
    inifile/inifile.cpp \
    log/tinylog.cpp \
    log/tlog.c \
    main/main.cpp \
    main/oam_cgi_client_demo.cpp \
    main/oam_cgi_server_demo.cpp \
    msg/msg.cpp \
    msg/server_demo_msg.cpp \
    network/communicator.cpp \
    network/epollserver.cpp \
    network/socket.cpp \
    network/tcpclient.cpp \
    network/tcpserver.cpp \
    network/tcpsocket.cpp \
    network/udpclient.cpp \
    network/udpserver.cpp \
    network/udpsocket.cpp \
    public/api.cpp \
    service/client_demo.cpp \
    service/common_communicator.cpp \
    service/common_server.cpp \
    service/server_demo.cpp \
    xml/tinyxml2.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
INCLUDEPATH += dev inifile log msg network xml rapidjson main service \
               public db lib d:/project/include/

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
    inifile/inifile.h \
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
    main/oam_cgi_client_demo.h \
    main/oam_cgi_server_demo.h \
    msg/msg.h \
    msg/server_demo_msg.h \
    network/communicator.h \
    network/epollserver.h \
    network/socket.h \
    network/tcpclient.h \
    network/tcpserver.h \
    network/tcpsocket.h \
    network/udpclient.h \
    network/udpserver.h \
    network/udpsocket.h \
    public/api.h \
    rapidjson/allocators.h \
    rapidjson/cursorstreamwrapper.h \
    rapidjson/document.h \
    rapidjson/encodedstream.h \
    rapidjson/encodings.h \
    rapidjson/error/en.h \
    rapidjson/error/error.h \
    rapidjson/filereadstream.h \
    rapidjson/filewritestream.h \
    rapidjson/fwd.h \
    rapidjson/internal/biginteger.h \
    rapidjson/internal/diyfp.h \
    rapidjson/internal/dtoa.h \
    rapidjson/internal/ieee754.h \
    rapidjson/internal/itoa.h \
    rapidjson/internal/meta.h \
    rapidjson/internal/pow10.h \
    rapidjson/internal/regex.h \
    rapidjson/internal/stack.h \
    rapidjson/internal/strfunc.h \
    rapidjson/internal/strtod.h \
    rapidjson/internal/swap.h \
    rapidjson/istreamwrapper.h \
    rapidjson/memorybuffer.h \
    rapidjson/memorystream.h \
    rapidjson/msinttypes/inttypes.h \
    rapidjson/msinttypes/stdint.h \
    rapidjson/ostreamwrapper.h \
    rapidjson/pointer.h \
    rapidjson/prettywriter.h \
    rapidjson/rapidjson.h \
    rapidjson/reader.h \
    rapidjson/schema.h \
    rapidjson/stream.h \
    rapidjson/stringbuffer.h \
    rapidjson/writer.h \
    service/client_demo.h \
    service/common_communicator.h \
    service/common_server.h \
    service/server_demo.h \
    xml/tinyxml2.h
