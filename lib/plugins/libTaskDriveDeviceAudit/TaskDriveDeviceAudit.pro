QT -= gui

TEMPLATE = lib
DEFINES += DRIVEDEVICEAUDIT_LIBRARY

CONFIG += c++11

#include($$PWD/../itaskplugin/ITaskPlugin.pri)

INCLUDEPATH += $$PWD/../ITaskPlugin
#win32:LIBS += -L$$OUT_PWD/../ITaskPlugin/debug -lITaskPlugin

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    TaskPlugin.cpp

HEADERS += \
    #TaskDriveDeviceAudit_global.h \
    TaskPlugin.h

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
