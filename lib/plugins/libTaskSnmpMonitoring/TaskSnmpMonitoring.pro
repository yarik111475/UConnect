QT -= gui

TEMPLATE = lib
DEFINES += SNMPMONITORING_LIBRARY

CONFIG += c++11

#include($$PWD/../itaskplugin/ITaskPlugin.pri)

INCLUDEPATH += $$PWD/../ITaskPlugin
#win32:LIBS += -L$$OUT_PWD/../ITaskPlugin/debug -lITaskPlugin

#------------------snmp++ 3.4.10 win----------
win32:INCLUDEPATH += $$PWD/../../../Lib/snmp++_win

CONFIG(debug, debug|release){
    win32:LIBS += -L$$OUT_PWD/../../../lib/snmp++_win/build/debug -lsnmp++
}
else{
    win32:LIBS += -L$$OUT_PWD/../../../lib/snmp++_win/build/release -lsnmp++
 }
#----------------------------------------------

#------------------openssl win msvc-----------
win32:INCLUDEPATH+= 'C:/Program Files/OpenSSL-Win64/include'
win32:LIBS += -L'C:/openssl_1_1_1k/1.1.1k/lib' -llibcrypto
win32:LIBS += -L'C:/openssl_1_1_1k/1.1.1k/lib' -llibssl
#--------------------------------------------------

#------------------snmp++ 3.4.10 unix----------
unix:INCLUDEPATH += $$PWD/../../../Lib/snmp++_unix
unix:CONFIG(debug, debug|release){
    LIBS += -L$$OUT_PWD/../../../Lib/snmp++_unix/build/debug -lsnmp++
}
else{
    LIBS += -L$$OUT_PWD/../../../Lib/snmp++_unix/build/release -lsnmp++
}
#-----------------------------------------

#-----------------windows libs------------
win32:LIBS += -lws2_32
#-----------------------------------------

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    TaskPlugin.cpp

HEADERS += \
    TaskItem.h \
    TaskPlugin.h
    #TaskSnmpMonitoring_global.h \

CONFIG(debug, debug|release) {
    DESTDIR = $$OUT_PWD/../../../../UConnect_build/AgentService_debug/plugins
} else {
    DESTDIR = $$OUT_PWD/../../../../UConnect_build/AgentService_release/plugins
}

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
