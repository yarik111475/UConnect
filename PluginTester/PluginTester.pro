QT += core gui widgets

CONFIG += c++11 console
#CONFIG -= app_bundle

#----------------custom include--------------
INCLUDEPATH += $$PWD/../Plugins/ITaskPlugin
#--------------------------------------------

CONFIG(debug, debug|release){
    #win32:LIBS += -L$$OUT_PWD/../Plugins/ITaskPlugin/debug -lITaskPlugin
} else{
    #win32:LIBS += -L$$OUT_PWD/../Plugins/ITaskPlugin/release -lITaskPlugin
}

unix:CONFIG(debug, debug|release){
    #LIBS += -L$$OUT_PWD/../Plugins/ITaskPlugin -lITaskPlugin
}else{
}

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp

CONFIG(debug, debug|release) {
    DESTDIR = $$OUT_PWD/../../../UConnect_build/AgentService_debug/bin
} else {
    DESTDIR = $$OUT_PWD/../../../UConnect_build/AgentService_release/bin
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

