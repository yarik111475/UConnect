QT += core gui widgets

TEMPLATE = lib
DEFINES += DISPLAYDEVICEAUDIT_LIBRARY

CONFIG += c++11

CONFIG += c++11

INCLUDEPATH += $$PWD/../ITaskPlugin
#win32:LIBS += -L$$OUT_PWD/../ITaskPlugin/debug -lITaskPlugin

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    TaskPlugin.cpp \
    win/WinUtils.cpp \
    win/Wmi.cpp

HEADERS += \
    #TaskDisplayDeviceAudit_global.h \
    TaskPlugin.h \
    win/WinUtils.h \
    win/Wmi.h

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
