#include <QCoreApplication>
#include <qdebug.h>
#include "T2Q_Client.h"


void  connection_status_callback(long nConnectionEvent
                            , long nExtendedErrorCode
                            , LPCSTR lpcstrInfoMessage)
{
    int i;
    i = nExtendedErrorCode;
    i += nExtendedErrorCode;
}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    TRANS2QUIK_CLIENT t2q;
    t2q.connectToHost(QHostAddress("192.168.0.102"), 50000);

    if (!t2q.waitForConnected(default_timeout)) {
                qDebug()<< t2q.errorString();
                return -1;
            }

    long ext_code;
    char err_buf[1000] ;
    long res = t2q.TRANS2QUIK_CONNECT ((LPSTR) "c:\\FinamJunior", &ext_code, err_buf, 1000);
    if (res != TRANS2QUIK_ALREADY_CONNECTED_TO_QUIK && res != TRANS2QUIK_SUCCESS)
        return -1;

    res = t2q.TRANS2QUIK_SET_CONNECTION_STATUS_CALLBACK( connection_status_callback, &ext_code, err_buf, 1000);
    //res = t2q.TRANS2QUIK_IS_QUIK_CONNECTED(&ext_code, err_buf, 1000);
    //long res = t2q.TRANS2QUIK_UNSUBSCRIBE_ORDERS();

    res++;

    return a.exec();
}

