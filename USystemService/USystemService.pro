QT += core network websockets

CONFIG += c++11 console

VERSION = 1.0.0.0
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

#-------------spdlog--------------
DEFINES += SPDLOG_COMPILED_LIB
INCLUDEPATH += $$PWD/../Lib/spdlog

win32:CONFIG(debug, debug|release){
    LIBS += -L$$OUT_PWD/../Lib/spdlog/debug -lspdlog
}
else{
    LIBS += -L$$OUT_PWD/../Lib/spdlog/release -lspdlog
}

unix:LIBS += -L$$OUT_PWD/../Lib/spdlog -lspdlog
#----------------------------------


include ($$PWD/../QtService/qtservice.pri)

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    network/HttpClient.cpp \
    network/TcpServer.cpp \
    network/WebsocketClient.cpp \
    service/LocalService.cpp \
    service/RemoteService.cpp \
    main.cpp \
    tasks/TaskExecutor.cpp \
    tasks/TaskQueue.cpp


HEADERS += \
    network/HttpClient.h \
    network/TcpServer.h \
    network/WebsocketClient.h \
    service/LocalService.h \
    service/RemoteService.h \
    tasks/Task.h \
    tasks/TaskExecutor.h \
    tasks/TaskQueue.h


TARGET = USystemService

CONFIG(debug, debug|release){
    DEFINES += DEBUG
} else{
    DEFINES += RELEASE
}

# Выбираем директорию сборки исполняемого файла
# в зависимости от режима сборки проекта
CONFIG(debug, debug|release) {
    DESTDIR = $$OUT_PWD/../../UConnect_build/USystemService_debug/bin
} else {
    DESTDIR = $$OUT_PWD/../../UConnect_build/USystemService_release/bin
}

# в зависимости от режима сборки проекта
# запускаем win deploy приложения в целевой директории, то есть собираем все dll
win32:CONFIG(debug, debug|release) {
    #QMAKE_POST_LINK = $$(QTDIR)/bin/windeployqt $$OUT_PWD/../../UConnect_build/USystemService_debug/bin
} else {
    #QMAKE_POST_LINK = $$(QTDIR)/bin/windeployqt $$OUT_PWD/../../UConnect_build/USystemService_release/bin
}

