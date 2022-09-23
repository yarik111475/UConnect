QT += core gui widgets network websockets

CONFIG += c++11 console

#include($$PWD/../Plugins/ITaskPlugin/ITaskPlugin.pri)
include ($$PWD/../../QtService/qtservice.pri)

INCLUDEPATH += $$PWD/../Plugins/ITaskPlugin

#-------------spdlog--------------
DEFINES += SPDLOG_COMPILED_LIB
INCLUDEPATH += $$PWD/../../Lib/spdlog

win32:CONFIG(debug, debug|release){
    LIBS += -L$$OUT_PWD/../../Lib/spdlog/debug -lspdlog
}
else{
    LIBS += -L$$OUT_PWD/../../Lib/spdlog/release -lspdlog
}

unix:CONFIG(debug, debug|release){
    LIBS += -L$$OUT_PWD/../../Lib/spdlog -lspdlog
}
else{
    LIBS += -L$$OUT_PWD/../../Lib/spdlog -lspdlog
}
#----------------------------------

#--------------openssl win-------------
win32:INCLUDEPATH += $$PWD/../../3rdparty/openssl/include

win32:LIBS += -L$$PWD/../../3rdparty/openssl/lib -llibssl
win32:LIBS += -L$$PWD/../../3rdparty/openssl/lib -llibcrypto
#---------------------------------------

#---------------openssl unix------------
unix:INCLUDEPATH += /usr/include/openssl

unix:LIBS += -lssl
unix:LIBS += -lcrypto
unix:LIBS += -lpthread
#---------------------------------------


## You can make your code fail to compile if it uses deprecated APIs.
## In order to do so, uncomment the following line.
##DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        Bootstrap.cpp \
        crypto/X509Csr.cpp \
        main.cpp \
        network/HttpClientBase.cpp \
        network/HttpClientCert.cpp \
        network/HttpClientTask.cpp \
        service/LocalService.cpp \
        task/TaskExecutor.cpp \
        task/TaskQueue.cpp \
        utils/Utils.cpp

HEADERS += \
    Bootstrap.h \
    crypto/X509Csr.h \
    network/HttpClientBase.h \
    network/HttpClientCert.h \
    network/HttpClientTask.h \
    service/LocalService.h \
    task/Task.h \
    task/TaskExecutor.h \
    task/TaskQueue.h \
    utils/Utils.h

CONFIG(debug, debug|release){
    DEFINES += DEBUG
} else{
    DEFINES += RELEASE
}

# Выбираем директорию сборки исполняемого файла
# в зависимости от режима сборки проекта
CONFIG(debug, debug|release) {
    DESTDIR = $$OUT_PWD/../../../UConnect_build/AgentService_debug/bin
} else {
    DESTDIR = $$OUT_PWD/../../../UConnect_build/AgentService_release/bin
}

# в зависимости от режима сборки проекта
# запускаем win deploy приложения в целевой директории, то есть собираем все dll
win32:CONFIG(debug, debug|release) {
    #QMAKE_POST_LINK = $$(QTDIR)/bin/windeployqt $$OUT_PWD/../../../UConnect_build/AgentService_debug/bin
} else {
    #QMAKE_POST_LINK = $$(QTDIR)/bin/windeployqt $$OUT_PWD/../../../UConnect_build/AgentService_release/bin
}

TARGET = AgentService
