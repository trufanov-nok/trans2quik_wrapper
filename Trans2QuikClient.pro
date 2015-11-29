QT += core network
QT -= gui

TARGET = Trans2QuikClient
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

win32:contains(QMAKE_HOST.arch, x86_64){
INCLUDEPATH += "C:\Trans2QuikAPI_1.3"
} else {
INCLUDEPATH += "C:\Trans2QuikAPI"
}

SOURCES += \
    T2Q_Client.cpp \
    main_client.cpp

HEADERS += \
    func_codes.h \
    T2Q_Client.h \
    io_utils.h


