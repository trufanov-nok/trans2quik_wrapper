QT += core network
QT -= gui

TARGET = Trans2QuikServer
CONFIG += console
CONFIG -= app_bundle
QMAKE_CXXFLAGS += -std=c++11

TEMPLATE = app

install_it.path = $$OUT_PWD

win32:contains(QMAKE_HOST.arch, x86_64){
INCLUDEPATH += "C:\Trans2QuikAPI_1.3"
LIBS += -ltrans2quik -L"C:\Trans2QuikAPI_1.3"
install_it.files = "C:\Trans2QuikAPI_1.3\trans2quik.dll"
} else {
INCLUDEPATH += "C:\Trans2QuikAPI"
LIBS += -ltrans2quik -L"C:\Trans2QuikAPI"
install_it.files = "C:\Trans2QuikAPI\TRANS2QUIK.dll"
}

INSTALLS += \
    install_it

SOURCES += main_server.cpp \
    T2Q_Server.cpp

HEADERS += \
    func_codes.h \
    T2Q_Server.h \
    io_utils.h \
    min_trans2quik_api_def.h

