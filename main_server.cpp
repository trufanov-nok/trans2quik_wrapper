#include <QCoreApplication>
#include "T2Q_server.h"
#include <qdebug.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    TRANS2QUIK_SERVER server;
    QHostAddress addr = QHostAddress("192.168.0.102");
//    addr = QHostAddress("0.0.0.0");
    if (!server.listen(addr,50000))
    {
        qDebug() << server.errorString();
        return -1;
    }
    return a.exec();
}

