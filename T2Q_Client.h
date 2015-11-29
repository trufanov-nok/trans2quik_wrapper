#ifndef WRAPPER_H
#define WRAPPER_H
#include <QHostAddress>
#include <QTcpSocket>
#include <QDataStream>
#include <QEventLoop>

#include "min_trans2quik_api_def.h"
#include "func_codes.h"
#include "io_utils.h"


const int default_timeout = 5 * 1000;

class TRANS2QUIK_CLIENT: public QTcpSocket
{
    Q_OBJECT
    TRANS2QUIK_CONNECTION_STATUS_CALLBACK _pfConnectionStatusCallback = nullptr;
    TRANS2QUIK_TRANSACTION_REPLY_CALLBACK _pfTransactionReplyCallback = nullptr;
    TRANS2QUIK_ORDER_STATUS_CALLBACK _pfnOrderStatusCallback = nullptr;
    TRANS2QUIK_TRADE_STATUS_CALLBACK _pfnTradeStatusCallback = nullptr;

    QByteArray _buf;
    QDataStream _out;

    QEventLoop _wait_loop;


    bool _wait_func_call_result = false;
    bool CallbackIfNeeded(func_codes fcode);


    long LPSTR_in_ExtErr_out(func_codes fc, LPSTR str_in, long* pnExtendedErrorCode, LPSTR lpstErrorMessage, DWORD dwErrorMessageSize);
    long ExtErr_out(func_codes fc, long* pnExtendedErrorCode, LPSTR lpstErrorMessage, DWORD dwErrorMessageSize);
    long Int_in_ExtErr_out(func_codes fc, quint32 int_in, long* pnExtendedErrorCode, LPSTR lpstErrorMessage, DWORD dwErrorMessageSize);
    long Emty_in_Empty_out(func_codes fc);
    long Int_in_Empty_out(func_codes fc, Descriptor int_in);
    double Int_in_Double_out(func_codes fc, Descriptor int_in);
    long TwoInt_in_Empty_out(func_codes fc, Descriptor int_in1, quint32 int_in2);
    FILETIME Int_in_FILETIME_out(func_codes fc, quint32 int_in);
    LPTSTR Int_in_LPTSTR_out(func_codes fc, quint32 int_in);
    long TwoLPSTR_in_Empty_out(func_codes fc, LPSTR a, LPSTR b);

public slots:
    void WaitRead();
public:
    TRANS2QUIK_CLIENT();
    QDataStream& data();
    void send();

    // better be private
    void WaitReadMessage(func_codes fcode);
    void WaitSendMessage();
    QDataStream _in;
    //

    long TRANS2QUIK_SEND_SYNC_TRANSACTION (LPSTR lpstTransactionString, long* pnReplyCode, PDWORD pdwTransId, EntityNumber* pnOrderNum, LPSTR lpstrResultMessage, DWORD dwResultMessageSize, long* pnExtendedErrorCode, LPSTR lpstErrorMessage, DWORD dwErrorMessageSize);
    long TRANS2QUIK_SEND_ASYNC_TRANSACTION (LPSTR lpstTransactionString, long* pnExtendedErrorCode, LPSTR lpstErrorMessage, DWORD dwErrorMessageSize);
    long TRANS2QUIK_CONNECT (LPSTR lpstConnectionParamsString, long* pnExtendedErrorCode, LPSTR lpstrErrorMessage, DWORD dwErrorMessageSize);
    long TRANS2QUIK_DISCONNECT (long* pnExtendedErrorCode, LPSTR lpstrErrorMessage, DWORD dwErrorMessageSize);
    long TRANS2QUIK_SET_CONNECTION_STATUS_CALLBACK (TRANS2QUIK_CONNECTION_STATUS_CALLBACK pfConnectionStatusCallback, long* pnExtendedErrorCode, LPSTR lpstrErrorMessage, DWORD dwErrorMessageSize);
    long TRANS2QUIK_SET_TRANSACTIONS_REPLY_CALLBACK (TRANS2QUIK_TRANSACTION_REPLY_CALLBACK pfTransactionReplyCallback, long* pnExtendedErrorCode, LPSTR lpstrErrorMessage, DWORD dwErrorMessageSize);
    long TRANS2QUIK_IS_QUIK_CONNECTED (long* pnExtendedErrorCode, LPSTR lpstrErrorMessage, DWORD dwErrorMessageSize);
    long TRANS2QUIK_IS_DLL_CONNECTED (long* pnExtendedErrorCode, LPSTR lpstrErrorMessage, DWORD dwErrorMessageSize);

    long TRANS2QUIK_SUBSCRIBE_ORDERS (LPSTR ClassCode, LPSTR Seccodes);
    long TRANS2QUIK_UNSUBSCRIBE_ORDERS ();
    long TRANS2QUIK_START_ORDERS (TRANS2QUIK_ORDER_STATUS_CALLBACK pfnOrderStatusCallback);

    Quantity TRANS2QUIK_ORDER_QTY (Descriptor orderDescriptor);
    long TRANS2QUIK_ORDER_DATE (Descriptor orderDescriptor);
    long TRANS2QUIK_ORDER_TIME (Descriptor orderDescriptor);
    long TRANS2QUIK_ORDER_ACTIVATION_TIME (Descriptor orderDescriptor);
    long TRANS2QUIK_ORDER_WITHDRAW_TIME (Descriptor orderDescriptor);
    long TRANS2QUIK_ORDER_EXPIRY (Descriptor orderDescriptor);
    double TRANS2QUIK_ORDER_ACCRUED_INT (Descriptor orderDescriptor);
    double TRANS2QUIK_ORDER_YIELD (Descriptor orderDescriptor);
    long TRANS2QUIK_ORDER_UID (Descriptor orderDescriptor);

    Quantity  TRANS2QUIK_ORDER_VISIBLE_QTY (Descriptor orderDescriptor);
    long TRANS2QUIK_ORDER_PERIOD (Descriptor orderDescriptor);
    FILETIME TRANS2QUIK_ORDER_FILETIME (Descriptor orderDescriptor);
    long TRANS2QUIK_ORDER_DATE_TIME (Descriptor orderDescriptor, long nTimeType);
    FILETIME TRANS2QUIK_ORDER_WITHDRAW_FILETIME (Descriptor orderDescriptor);

#if defined(_WIN64) || defined(_64bit)
    long TRANS2QUIK_ORDER_VALUE_ENTRY_TYPE(Descriptor orderDescriptor);
    long TRANS2QUIK_ORDER_EXTENDED_FLAGS(Descriptor orderDescriptor);
    Quantity  TRANS2QUIK_ORDER_MIN_QTY (Descriptor orderDescriptor);
    long TRANS2QUIK_ORDER_EXEC_TYPE(Descriptor orderDescriptor);
    double TRANS2QUIK_ORDER_AWG_PRICE (Descriptor orderDescriptor);
#endif

    LPTSTR TRANS2QUIK_ORDER_USERID (Descriptor orderDescriptor);
    LPTSTR TRANS2QUIK_ORDER_ACCOUNT (Descriptor orderDescriptor);
    LPTSTR TRANS2QUIK_ORDER_BROKERREF (Descriptor orderDescriptor);
    LPTSTR TRANS2QUIK_ORDER_CLIENT_CODE (Descriptor orderDescriptor);
    LPTSTR TRANS2QUIK_ORDER_FIRMID (Descriptor orderDescriptor);
#if defined(_WIN64) || defined(_64bit)
    LPTSTR TRANS2QUIK_ORDER_REJECT_REASON (Descriptor orderDescriptor);
#endif


    long TRANS2QUIK_SUBSCRIBE_TRADES (LPSTR ClassCode, LPSTR Seccodes);
    long TRANS2QUIK_UNSUBSCRIBE_TRADES ();
    long TRANS2QUIK_START_TRADES(TRANS2QUIK_TRADE_STATUS_CALLBACK pfnTradeStatusCallback);

    long TRANS2QUIK_TRADE_DATE (Descriptor tradeDescriptor);
    long TRANS2QUIK_TRADE_SETTLE_DATE (Descriptor tradeDescriptor);
    long TRANS2QUIK_TRADE_TIME (Descriptor tradeDescriptor);
    long TRANS2QUIK_TRADE_IS_MARGINAL (Descriptor tradeDescriptor);
    double TRANS2QUIK_TRADE_ACCRUED_INT (Descriptor tradeDescriptor);
    double TRANS2QUIK_TRADE_YIELD (Descriptor tradeDescriptor);
    double TRANS2QUIK_TRADE_TS_COMMISSION (Descriptor tradeDescriptor);
    double TRANS2QUIK_TRADE_CLEARING_CENTER_COMMISSION (Descriptor tradeDescriptor);
    double TRANS2QUIK_TRADE_EXCHANGE_COMMISSION (Descriptor tradeDescriptor);
    double TRANS2QUIK_TRADE_TRADING_SYSTEM_COMMISSION (Descriptor tradeDescriptor);
    double TRANS2QUIK_TRADE_PRICE2 (Descriptor tradeDescriptor);
    double TRANS2QUIK_TRADE_REPO_RATE (Descriptor tradeDescriptor);
    double TRANS2QUIK_TRADE_REPO_VALUE (Descriptor tradeDescriptor);
    double TRANS2QUIK_TRADE_REPO2_VALUE (Descriptor tradeDescriptor);
    double TRANS2QUIK_TRADE_ACCRUED_INT2 (Descriptor tradeDescriptor);
    long TRANS2QUIK_TRADE_REPO_TERM (Descriptor tradeDescriptor);
    double TRANS2QUIK_TRADE_START_DISCOUNT (Descriptor tradeDescriptor);
    double TRANS2QUIK_TRADE_LOWER_DISCOUNT (Descriptor tradeDescriptor);
    double TRANS2QUIK_TRADE_UPPER_DISCOUNT (Descriptor tradeDescriptor);
    long TRANS2QUIK_TRADE_BLOCK_SECURITIES (Descriptor tradeDescriptor);
    long TRANS2QUIK_TRADE_PERIOD (Descriptor tradeDescriptor);
    long TRANS2QUIK_TRADE_KIND (Descriptor tradeDescriptor);

    FILETIME TRANS2QUIK_TRADE_FILETIME (Descriptor tradeDescriptor);
    long TRANS2QUIK_TRADE_DATE_TIME (Descriptor tradeDescriptor, long nTimeType);
#if defined(_WIN64) || defined(_64bit)
    double TRANS2QUIK_TRADE_BROKER_COMMISSION (Descriptor tradeDescriptor);
    long TRANS2QUIK_TRADE_TRANSID (Descriptor tradeDescriptor);
#endif

    LPTSTR TRANS2QUIK_TRADE_CURRENCY (Descriptor tradeDescriptor);
    LPTSTR TRANS2QUIK_TRADE_SETTLE_CURRENCY (Descriptor tradeDescriptor);
    LPTSTR TRANS2QUIK_TRADE_SETTLE_CODE (Descriptor tradeDescriptor);
    LPTSTR TRANS2QUIK_TRADE_ACCOUNT (Descriptor tradeDescriptor);
    LPTSTR TRANS2QUIK_TRADE_BROKERREF (Descriptor tradeDescriptor);
    LPTSTR TRANS2QUIK_TRADE_CLIENT_CODE (Descriptor tradeDescriptor);
    LPTSTR TRANS2QUIK_TRADE_USERID (Descriptor tradeDescriptor);
    LPTSTR TRANS2QUIK_TRADE_FIRMID (Descriptor tradeDescriptor);
    LPTSTR TRANS2QUIK_TRADE_PARTNER_FIRMID (Descriptor tradeDescriptor);
    LPTSTR TRANS2QUIK_TRADE_EXCHANGE_CODE (Descriptor tradeDescriptor);
    LPTSTR TRANS2QUIK_TRADE_STATION_ID (Descriptor tradeDescriptor);

#if defined(_WIN64) || defined(_64bit)
    LPTSTR TRANS2QUIK_TRANSACTION_REPLY_CLASS_CODE (Descriptor tradeDescriptor);
    LPTSTR TRANS2QUIK_TRANSACTION_REPLY_SEC_CODE (Descriptor tradeDescriptor);
    double TRANS2QUIK_TRANSACTION_REPLY_PRICE (Descriptor tradeDescriptor);
    Quantity TRANS2QUIK_TRANSACTION_REPLY_QUANTITY (Descriptor tradeDescriptor);
    Quantity TRANS2QUIK_TRANSACTION_REPLY_BALANCE (Descriptor tradeDescriptor);
    LPTSTR TRANS2QUIK_TRANSACTION_REPLY_FIRMID (Descriptor tradeDescriptor);
    LPTSTR TRANS2QUIK_TRANSACTION_REPLY_ACCOUNT (Descriptor tradeDescriptor);
    LPTSTR TRANS2QUIK_TRANSACTION_REPLY_CLIENT_CODE (Descriptor tradeDescriptor);
    LPTSTR TRANS2QUIK_TRANSACTION_REPLY_BROKERREF (Descriptor tradeDescriptor);
    LPTSTR TRANS2QUIK_TRANSACTION_REPLY_EXCHANGE_CODE (Descriptor tradeDescriptor);
#endif
};

#endif // WRAPPER_H
