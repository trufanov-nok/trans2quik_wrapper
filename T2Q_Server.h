#ifndef T2Q_SERVER_H
#define T2Q_SERVER_H
#include <Windows.h>
#include <QTcpServer>
#include <QTcpSocket>
#include <QEventLoop>
#include <QDataStream>
#include <QPointer>
#include "func_codes.h"
#include "trans2quik_api.h"

typedef long (*T2Q_ExtErr_Long) (long* , LPSTR , DWORD );
typedef long (*T2Q_TwoLPSTR_Long) (LPSTR , LPSTR);
typedef long (*T2Q_Empty_Long) ();

typedef OrderDescriptor Descriptor; // they all the same type



class TRANS2QUIK_SERVER : public QTcpServer
{
    Q_OBJECT
    QPointer<QTcpSocket> curConnection;
    QByteArray _buf;
    QDataStream _out;

    QEventLoop _wait_loop;


    void Call_ExtError_ret_long(func_codes fc, T2Q_ExtErr_Long f);
    void Call_TwoLPSTR_ret_long(func_codes fc, T2Q_TwoLPSTR_Long f);
    void Call_Emty_ret_long(func_codes fc, T2Q_Empty_Long f);
    template<class A>
    void SendRes(func_codes fc, A res)
    {
        PrepareMessage<A>(data(), fc, res);
        curConnection->write(_buf);
        curConnection->flush();
    }

    void SendFILETIME(func_codes fc, FILETIME res);
    void SendLPTSTR(func_codes fc, LPTSTR res);


    static TRANS2QUIK_SERVER* _srv;
    static void _TRANS2QUIK_CONNECTION_STATUS_CALLBACK (long nConnectionEvent, long nExtendedErrorCode, LPCSTR lpcstrInfoMessage);
#if defined(_WIN64) || defined(_64bit)
    static void _TRANS2QUIK_TRANSACTION_REPLY_CALLBACK (long nTransactionResult, long nTransactionExtendedErrorCode, long nTransactionReplyCode, DWORD dwTransId, EntityNumber OrderNum, PCSTR lpcstrTransactionReplyMessage, TransactionReplyDescriptor transReplyDescriptor);
    static void _TRANS2QUIK_ORDER_STATUS_CALLBACK      (long nMode, DWORD dwTransID, EntityNumber dNumber, LPCSTR ClassCode, LPCSTR SecCode, double dPrice, Quantity  nBalance, double dValue, long nIsSell, long nStatus, OrderDescriptor orderDescriptor);
    static void _TRANS2QUIK_TRADE_STATUS_CALLBACK      (long nMode, EntityNumber dNumber, EntityNumber dOrderNumber, LPCSTR ClassCode, LPCSTR SecCode, double dPrice, Quantity nQty, double dValue, long nIsSell, TradeDescriptor tradeDescriptor);
#else
    static void _TRANS2QUIK_TRANSACTION_REPLY_CALLBACK (long nTransactionResult, long nTransactionExtendedErrorCode, long nTransactionReplyCode, DWORD dwTransId, EntityNumber OrderNum, LPCSTR lpcstrTransactionReplyMessage);
    static void _TRANS2QUIK_ORDER_STATUS_CALLBACK      (long nMode, DWORD dwTransID, double dNumber, LPCSTR ClassCode, LPCSTR SecCode, double dPrice, long nBalance, double dValue, long nIsSell, long nStatus, long orderDescriptor);
    static void _TRANS2QUIK_TRADE_STATUS_CALLBACK      (long nMode, double dNumber, double dOrderNumber, LPCSTR ClassCode, LPCSTR SecCode, double dPrice, long nQty, double dValue, long nIsSell, long tradeDescriptor);
#endif

    QDataStream &data();

    void SetCallback(func_codes& fc);
private slots:
    void clientConnected();
    void processMessages();
public:
    QDataStream _in;
    TRANS2QUIK_SERVER();
};

#endif // T2Q_SERVER_H
