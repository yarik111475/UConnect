CONFIG -= qt

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++11

win32:INCLUDEPATH += C:/openssl_1_1_1k/1.1.1k/include

#win32:LIBS += -lws2_32
#win32:LIBS += -L'C:/Program Files/OpenSSL-Win64/lib' -llibcrypto
#win32:LIBS += -L'C:/Program Files/OpenSSL-Win64/lib' -llibssl

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    IPv6Utility.cpp \
    address.cpp \
    asn1.cpp \
    auth_priv.cpp \
    counter.cpp \
    ctr64.cpp \
    eventlist.cpp \
    eventlistholder.cpp \
    gauge.cpp \
    idea.cpp \
    integer.cpp \
    log.cpp \
    md5c.cpp \
    mp_v3.cpp \
    msec.cpp \
    msgqueue.cpp \
    notifyqueue.cpp \
    octet.cpp \
    oid.cpp \
    pdu.cpp \
    reentrant.cpp \
    sha.cpp \
    snmpmsg.cpp \
    target.cpp \
    timetick.cpp \
    userdefined.cpp \
    usertimeout.cpp \
    usm_v3.cpp \
    uxsnmp.cpp \
    v3.cpp \
    vb.cpp

HEADERS += \
    snmp_pp/IPv6Utility.h \
    snmp_pp/address.h \
    snmp_pp/asn1.h \
    snmp_pp/auth_priv.h \
    snmp_pp/collect.h \
    snmp_pp/config.h \
    snmp_pp/config_snmp_pp.h \
    snmp_pp/config_snmp_pp.h.in \
    snmp_pp/counter.h \
    snmp_pp/ctr64.h \
    snmp_pp/eventlist.h \
    snmp_pp/eventlistholder.h \
    snmp_pp/gauge.h \
    snmp_pp/idea.h \
    snmp_pp/integer.h \
    snmp_pp/libsnmp.h.in \
    snmp_pp/log.h \
    snmp_pp/md5.h \
    snmp_pp/mp_v3.h \
    snmp_pp/msec.h \
    snmp_pp/msgqueue.h \
    snmp_pp/notifyqueue.h \
    snmp_pp/octet.h \
    snmp_pp/oid.h \
    snmp_pp/oid_def.h \
    snmp_pp/pdu.h \
    snmp_pp/reentrant.h \
    snmp_pp/sha.h \
    snmp_pp/smi.h \
    snmp_pp/smival.h \
    snmp_pp/snmp_pp.h \
    snmp_pp/snmperrs.h \
    snmp_pp/snmpmsg.h \
    snmp_pp/target.h \
    snmp_pp/timetick.h \
    snmp_pp/userdefined.h \
    snmp_pp/usertimeout.h \
    snmp_pp/usm_v3.h \
    snmp_pp/uxsnmp.h \
    snmp_pp/v3.h \
    snmp_pp/vb.h \
    libsnmp.h \
    snmp_pp.h


CONFIG(debug, debug|release){
    DESTDIR += $$OUT_PWD/build/debug
} else{
    DESTDIR += $$OUT_PWD/build/release
}

TARGET = snmp++
