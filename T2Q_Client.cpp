#include "T2Q_Client.h"

bool TRANS2QUIK_CLIENT::CallbackIfNeeded(func_codes fcode)
{
    switch(fcode)
    {
    case E_TRANS2QUIK_CONNECTION_STATUS_CALLBACK:
        if(_pfConnectionStatusCallback)
        {
            quint32 dummy;
            _in >> dummy;
            long nConnectionEvent = dummy;
            _in >> dummy;
            long nExtendedErrorCode = dummy;
            LPSTR lpcstrInfoMessage = ReadAndCreate_LPSTR_from_QString(_in);
            _pfConnectionStatusCallback(nConnectionEvent, nExtendedErrorCode, (LPCSTR) lpcstrInfoMessage);
            if (lpcstrInfoMessage) free(lpcstrInfoMessage);
        }
        break;
    case E_TRANS2QUIK_TRANSACTION_REPLY_CALLBACK:
        if(_pfTransactionReplyCallback)
        {
            quint32 dummy;
            _in >> dummy; long nTransactionResult = dummy;
            _in >> dummy; long nTransactionExtendedErrorCode = dummy;
            _in >> dummy; long nTransactionReplyCode = dummy;
            _in >> dummy; DWORD dwTransId = dummy;
            _in >> dummy; EntityNumber nOrderNum = dummy;
            LPSTR lpcstrTransactionReplyMessage = ReadAndCreate_LPSTR_from_QString(_in);
#if defined(_WIN64) || defined(_64bit)
            _in >> dummy; Descriptor transReplyDescriptor = dummy;
            _pfTransactionReplyCallback(nTransactionResult, nTransactionExtendedErrorCode, nTransactionReplyCode,
                                        dwTransId, nOrderNum, (LPCSTR) lpcstrTransactionReplyMessage, transReplyDescriptor);
#else
            _pfTransactionReplyCallback(nTransactionResult, nTransactionExtendedErrorCode, nTransactionReplyCode,
                                        dwTransId, nOrderNum, (LPCSTR) lpcstrTransactionReplyMessage);
#endif

            if (lpcstrTransactionReplyMessage) free(lpcstrTransactionReplyMessage);
        }
        break;
    case E_TRANS2QUIK_ORDER_STATUS_CALLBACK:
        if(_pfnOrderStatusCallback)
        {
            quint32 dummy;
            _in >> dummy; long nMode = dummy;
            _in >> dummy; DWORD dwTransID = dummy;
            _in >> dummy; EntityNumber dNumber = dummy;
            LPSTR ClassCode = ReadAndCreate_LPSTR_from_QString(_in);
            LPSTR SecCode = ReadAndCreate_LPSTR_from_QString(_in);
            _in >> dummy; double dPrice = dummy;
            _in >> dummy; Quantity  nBalance = dummy;
            _in >> dummy; double dValue = dummy;
            _in >> dummy; long nIsSell = dummy;
            _in >> dummy; long nStatus = dummy;
            _in >> dummy; Descriptor orderDescriptor = dummy;
            _pfnOrderStatusCallback(nMode, dwTransID, dNumber, (LPCSTR) ClassCode, (LPCSTR) SecCode, dPrice, nBalance,
                                        dValue, nIsSell, nStatus, orderDescriptor);
            if (ClassCode) free(ClassCode);
            if (SecCode) free(SecCode);
        }
        break;
    case E_TRANS2QUIK_TRADE_STATUS_CALLBACK:
        if(_pfnTradeStatusCallback)
        {
            quint32 dummy;
            _in >> dummy; long nMode = dummy;
            _in >> dummy; EntityNumber dNumber = dummy;
            _in >> dummy; EntityNumber dOrderNumber = dummy;
            LPSTR ClassCode = ReadAndCreate_LPSTR_from_QString(_in);
            LPSTR SecCode = ReadAndCreate_LPSTR_from_QString(_in);
            _in >> dummy; double dPrice = dummy;
            _in >> dummy; Quantity  nQty = dummy;
            _in >> dummy; double dValue = dummy;
            _in >> dummy; long nIsSell = dummy;
            _in >> dummy; Descriptor tradeDescriptor = dummy;
            _pfnTradeStatusCallback(nMode, dNumber, dOrderNumber, (LPCSTR) ClassCode, (LPCSTR) SecCode, dPrice, nQty,
                                        dValue, nIsSell, tradeDescriptor);
            if (ClassCode) free(ClassCode);
            if (SecCode) free(SecCode);
        }
        break;
    default:
        return false;
    }
    return true;
}


void TRANS2QUIK_CLIENT::WaitSendMessage()
{
    write(_buf);
    flush();
}

void TRANS2QUIK_CLIENT::WaitRead()
{
    //only callbacks are processed here
        if (_wait_func_call_result) return; // some func already waits for this messsage
    func_codes code;

        while (bytesAvailable() < (int)sizeof(quint32)) {
            _wait_loop.exec();
        }

        quint32 dummy;
        _in >> dummy;

        while (bytesAvailable() < dummy) {
            _wait_loop.exec();
        }

        _in >> dummy;
        code = (func_codes) dummy;

        if (!CallbackIfNeeded(code))
            Q_ASSERT(false);
}

void TRANS2QUIK_CLIENT::WaitReadMessage(func_codes fcode)
{
    func_codes code;
    _wait_func_call_result = true;
    do
    {

        _wait_loop.exec();

        while (bytesAvailable() < (int)sizeof(quint32)) {
            _wait_loop.exec();
        }

        quint32 dummy;
        _in >> dummy;

        while (bytesAvailable() < dummy) {
            _wait_loop.exec();
        }

        _in >> dummy;
        code = (func_codes) dummy;

        if (!CallbackIfNeeded(code))
            Q_ASSERT(code == fcode);
    } while (code != fcode);
    _wait_func_call_result = false;
}



TRANS2QUIK_CLIENT::TRANS2QUIK_CLIENT(): _buf(), _out(&_buf, QIODevice::WriteOnly)
{
     QObject::connect(this, SIGNAL(readyRead()), &_wait_loop, SLOT(quit()));
     QObject::connect(this, SIGNAL(readyRead()), this,  SLOT(WaitRead()));
     _in.setDevice(this);
     _in.setVersion(QDataStream::Qt_4_0);
     _out.setVersion(QDataStream::Qt_4_0);
}

QDataStream& TRANS2QUIK_CLIENT::data() {
    _buf.clear();
    _out.device()->seek(0);
    return _out;
}

void TRANS2QUIK_CLIENT::send()
{
    write(_buf);
}

long TRANS2QUIK_CLIENT::TRANS2QUIK_SEND_SYNC_TRANSACTION (LPSTR lpstTransactionString, long* pnReplyCode, PDWORD pdwTransId, EntityNumber* pnOrderNum, LPSTR lpstrResultMessage, DWORD dwResultMessageSize, long* pnExtendedErrorCode, LPSTR lpstErrorMessage, DWORD dwErrorMessageSize)
{
    PrepareMessage<QString, quint32, quint32>(data(), E_TRANS2QUIK_SEND_SYNC_TRANSACTION, QString(lpstTransactionString), dwResultMessageSize, dwErrorMessageSize);
    WaitSendMessage();
    WaitReadMessage(E_TRANS2QUIK_SEND_SYNC_TRANSACTION);

    quint32 res;
    _in >> res;

    ReadLongPtr(_in, pnReplyCode);
    ReadPDWORD(_in, pdwTransId);
    ReadEntityNumber(_in, pnOrderNum);
    ReadQString2LPSTR(_in, lpstrResultMessage);
    ReadLongPtr(_in, pnExtendedErrorCode);
    ReadQString2LPSTR(_in, lpstErrorMessage);

    return res;
}


long TRANS2QUIK_CLIENT::LPSTR_in_ExtErr_out(func_codes fc, LPSTR str_in, long* pnExtendedErrorCode, LPSTR lpstErrorMessage, DWORD dwErrorMessageSize)
{
    PrepareMessage<QString, quint32>(data(), fc, QString(str_in), dwErrorMessageSize);
    WaitSendMessage();
    WaitReadMessage(fc);

    quint32 res;
    _in >> res;

    ReadLongPtr(_in, pnExtendedErrorCode);
    ReadQString2LPSTR(_in, lpstErrorMessage);

    return res;
}

long TRANS2QUIK_CLIENT::TRANS2QUIK_SEND_ASYNC_TRANSACTION (LPSTR lpstTransactionString, long* pnExtendedErrorCode, LPSTR lpstErrorMessage, DWORD dwErrorMessageSize)
{
  return LPSTR_in_ExtErr_out(E_TRANS2QUIK_SEND_ASYNC_TRANSACTION, lpstTransactionString, pnExtendedErrorCode, lpstErrorMessage, dwErrorMessageSize);
}

long TRANS2QUIK_CLIENT::TRANS2QUIK_CONNECT (LPSTR lpstConnectionParamsString, long* pnExtendedErrorCode, LPSTR lpstrErrorMessage, DWORD dwErrorMessageSize)
{
  return LPSTR_in_ExtErr_out(E_TRANS2QUIK_CONNECT, lpstConnectionParamsString, pnExtendedErrorCode, lpstrErrorMessage, dwErrorMessageSize);
}

long TRANS2QUIK_CLIENT::ExtErr_out(func_codes fc, long* pnExtendedErrorCode, LPSTR lpstErrorMessage, DWORD dwErrorMessageSize)
{
    PrepareMessage<quint32>(data(), fc, dwErrorMessageSize);
    WaitSendMessage();
    WaitReadMessage(fc);

    quint32 res;
    _in >> res;

    ReadLongPtr(_in, pnExtendedErrorCode);
    ReadQString2LPSTR(_in, lpstErrorMessage);

    return res;
}

long TRANS2QUIK_CLIENT::TRANS2QUIK_DISCONNECT (long* pnExtendedErrorCode, LPSTR lpstrErrorMessage, DWORD dwErrorMessageSize)
{
    return ExtErr_out(E_TRANS2QUIK_DISCONNECT, pnExtendedErrorCode, lpstrErrorMessage, dwErrorMessageSize);
}

long TRANS2QUIK_CLIENT::Int_in_ExtErr_out(func_codes fc, quint32 int_in, long* pnExtendedErrorCode, LPSTR lpstErrorMessage, DWORD dwErrorMessageSize)
{
    PrepareMessage<quint32,quint32>(data(), fc, int_in, dwErrorMessageSize);
    WaitSendMessage();
    WaitReadMessage(fc);

    quint32 res;
    _in >> res;

    ReadLongPtr(_in, pnExtendedErrorCode);
    ReadQString2LPSTR(_in, lpstErrorMessage);

    return res;
}

long TRANS2QUIK_CLIENT::TRANS2QUIK_SET_CONNECTION_STATUS_CALLBACK (TRANS2QUIK_CONNECTION_STATUS_CALLBACK pfConnectionStatusCallback, long* pnExtendedErrorCode, LPSTR lpstrErrorMessage, DWORD dwErrorMessageSize)
{
    _pfConnectionStatusCallback = pfConnectionStatusCallback;
    return Int_in_ExtErr_out(E_TRANS2QUIK_SET_CONNECTION_STATUS_CALLBACK, quint32(pfConnectionStatusCallback != NULL), pnExtendedErrorCode, lpstrErrorMessage, dwErrorMessageSize);
}

long TRANS2QUIK_CLIENT::TRANS2QUIK_SET_TRANSACTIONS_REPLY_CALLBACK (TRANS2QUIK_TRANSACTION_REPLY_CALLBACK pfTransactionReplyCallback, long* pnExtendedErrorCode, LPSTR lpstrErrorMessage, DWORD dwErrorMessageSize)
{
    _pfTransactionReplyCallback = pfTransactionReplyCallback;
    return Int_in_ExtErr_out(E_TRANS2QUIK_TRANSACTION_REPLY_CALLBACK, quint32(pfTransactionReplyCallback != NULL), pnExtendedErrorCode, lpstrErrorMessage, dwErrorMessageSize);

}

long TRANS2QUIK_CLIENT::TRANS2QUIK_IS_QUIK_CONNECTED (long* pnExtendedErrorCode, LPSTR lpstrErrorMessage, DWORD dwErrorMessageSize)
{
    return ExtErr_out(E_TRANS2QUIK_IS_QUIK_CONNECTED, pnExtendedErrorCode, lpstrErrorMessage, dwErrorMessageSize);
}

long TRANS2QUIK_CLIENT::TRANS2QUIK_IS_DLL_CONNECTED (long* pnExtendedErrorCode, LPSTR lpstrErrorMessage, DWORD dwErrorMessageSize)
{
    return ExtErr_out(E_TRANS2QUIK_IS_DLL_CONNECTED, pnExtendedErrorCode, lpstrErrorMessage, dwErrorMessageSize);
}

long TRANS2QUIK_CLIENT::TwoLPSTR_in_Empty_out(func_codes fc, LPSTR a, LPSTR b)
{
    PrepareMessage<QString, QString>(data(), fc, QString(a), QString(b));
    WaitSendMessage();
    WaitReadMessage(fc);

    quint32 res;
    _in >> res;

    return res;
}

long TRANS2QUIK_CLIENT::TRANS2QUIK_SUBSCRIBE_ORDERS (LPSTR ClassCode, LPSTR Seccodes)
{
    return TwoLPSTR_in_Empty_out(E_TRANS2QUIK_SUBSCRIBE_ORDERS, ClassCode, Seccodes);
}

long TRANS2QUIK_CLIENT::Emty_in_Empty_out(func_codes fc)
{
    PrepareMessage(data(), fc);
    WaitSendMessage();
    WaitReadMessage(fc);

    quint32 res;
    _in >> res;

    return res;
}

long TRANS2QUIK_CLIENT::TRANS2QUIK_UNSUBSCRIBE_ORDERS ()
{
    return Emty_in_Empty_out(E_TRANS2QUIK_UNSUBSCRIBE_ORDERS);
}

template<class R>
R Int_in_R_out(TRANS2QUIK_CLIENT& cl, func_codes fc, quint32 int_in)
{
    PrepareMessage<quint32>(cl.data(), fc, int_in);
    cl.WaitSendMessage();
    cl.WaitReadMessage(fc);

    R res;
    cl._in >> res;

    return res;
}

long TRANS2QUIK_CLIENT::Int_in_Empty_out(func_codes fc, Descriptor int_in)
{
    return (long) Int_in_R_out<Descriptor>(*this, fc, int_in);
}

double TRANS2QUIK_CLIENT::Int_in_Double_out(func_codes fc, Descriptor int_in)
{
    return (double) Int_in_R_out<Descriptor>(*this, fc, int_in);
}

long TRANS2QUIK_CLIENT::TwoInt_in_Empty_out(func_codes fc, Descriptor int_in1, quint32 int_in2)
{
    PrepareMessage<Descriptor, qint32>(data(), fc, int_in1, int_in2);
    WaitSendMessage();
    WaitReadMessage(fc);

    quint32 res;
    _in >> res;

    return res;
}

long TRANS2QUIK_CLIENT::TRANS2QUIK_START_ORDERS (TRANS2QUIK_ORDER_STATUS_CALLBACK pfnOrderStatusCallback)
{
    _pfnOrderStatusCallback = pfnOrderStatusCallback;
    return Int_in_Empty_out(E_TRANS2QUIK_ORDER_STATUS_CALLBACK, quint32(pfnOrderStatusCallback != NULL));
}


Quantity TRANS2QUIK_CLIENT::TRANS2QUIK_ORDER_QTY (Descriptor orderDescriptor)
{
    return (Quantity) Int_in_Empty_out(E_TRANS2QUIK_ORDER_QTY, orderDescriptor);
}

long TRANS2QUIK_CLIENT::TRANS2QUIK_ORDER_DATE (Descriptor orderDescriptor)
{
    return Int_in_Empty_out(E_TRANS2QUIK_ORDER_DATE, orderDescriptor);
}

long TRANS2QUIK_CLIENT::TRANS2QUIK_ORDER_TIME (Descriptor orderDescriptor)
{
    return Int_in_Empty_out(E_TRANS2QUIK_ORDER_TIME, orderDescriptor);
}

long TRANS2QUIK_CLIENT::TRANS2QUIK_ORDER_ACTIVATION_TIME (Descriptor orderDescriptor)
{
    return Int_in_Empty_out(E_TRANS2QUIK_ORDER_ACTIVATION_TIME, orderDescriptor);
}

long TRANS2QUIK_CLIENT::TRANS2QUIK_ORDER_WITHDRAW_TIME (Descriptor orderDescriptor)
{
    return Int_in_Empty_out(E_TRANS2QUIK_ORDER_WITHDRAW_TIME, orderDescriptor);
}

long TRANS2QUIK_CLIENT::TRANS2QUIK_ORDER_EXPIRY (Descriptor orderDescriptor)
{
    return Int_in_Empty_out(E_TRANS2QUIK_ORDER_EXPIRY, orderDescriptor);
}

double TRANS2QUIK_CLIENT::TRANS2QUIK_ORDER_ACCRUED_INT (Descriptor orderDescriptor)
{
    return Int_in_Empty_out(E_TRANS2QUIK_ORDER_ACCRUED_INT, orderDescriptor);
}

double TRANS2QUIK_CLIENT::TRANS2QUIK_ORDER_YIELD (Descriptor orderDescriptor)
{
    return Int_in_Empty_out(E_TRANS2QUIK_ORDER_YIELD, orderDescriptor);
}

long TRANS2QUIK_CLIENT::TRANS2QUIK_ORDER_UID (Descriptor orderDescriptor)
{
    return Int_in_Empty_out(E_TRANS2QUIK_ORDER_UID, orderDescriptor);
}


Quantity TRANS2QUIK_CLIENT::TRANS2QUIK_ORDER_VISIBLE_QTY (Descriptor orderDescriptor)
{
    return (Quantity) Int_in_Empty_out(E_TRANS2QUIK_ORDER_VISIBLE_QTY, orderDescriptor);
}

long TRANS2QUIK_CLIENT::TRANS2QUIK_ORDER_PERIOD (Descriptor orderDescriptor)
{
    return Int_in_Empty_out(E_TRANS2QUIK_ORDER_PERIOD, orderDescriptor);
}

FILETIME TRANS2QUIK_CLIENT::Int_in_FILETIME_out(func_codes fc, quint32 int_in)
{
    PrepareMessage<quint32>(data(), fc, int_in);
    WaitSendMessage();
    WaitReadMessage(fc);

    FILETIME res;
    quint32 dummy;
    _in >> dummy; res.dwHighDateTime = (DWORD)dummy;
    _in >> dummy; res.dwLowDateTime = (DWORD)dummy;

    return res;
}

FILETIME TRANS2QUIK_CLIENT::TRANS2QUIK_ORDER_FILETIME (Descriptor orderDescriptor)
{
    return Int_in_FILETIME_out(E_TRANS2QUIK_ORDER_FILETIME, orderDescriptor);
}

long TRANS2QUIK_CLIENT::TRANS2QUIK_ORDER_DATE_TIME (Descriptor orderDescriptor, long nTimeType)
{
    return TwoInt_in_Empty_out(E_TRANS2QUIK_ORDER_DATE_TIME, orderDescriptor, nTimeType);
}

FILETIME TRANS2QUIK_CLIENT::TRANS2QUIK_ORDER_WITHDRAW_FILETIME (Descriptor orderDescriptor)
{
    return Int_in_FILETIME_out(E_TRANS2QUIK_ORDER_WITHDRAW_FILETIME, orderDescriptor);
}


#if defined(_WIN64) || defined(_64bit)
long TRANS2QUIK_CLIENT::TRANS2QUIK_ORDER_VALUE_ENTRY_TYPE (Descriptor orderDescriptor)
{
    return Int_in_Empty_out(E_TRANS2QUIK_ORDER_VALUE_ENTRY_TYPE, orderDescriptor);
}

long TRANS2QUIK_CLIENT::TRANS2QUIK_ORDER_EXTENDED_FLAGS (Descriptor orderDescriptor)
{
    return Int_in_Empty_out(E_TRANS2QUIK_ORDER_EXTENDED_FLAGS, orderDescriptor);
}

Quantity TRANS2QUIK_CLIENT::TRANS2QUIK_ORDER_MIN_QTY (Descriptor orderDescriptor)
{
    return (Quantity) Int_in_Empty_out(E_TRANS2QUIK_ORDER_MIN_QTY, orderDescriptor);
}

long TRANS2QUIK_CLIENT::TRANS2QUIK_ORDER_EXEC_TYPE (Descriptor orderDescriptor)
{
    return Int_in_Empty_out(E_TRANS2QUIK_ORDER_EXEC_TYPE, orderDescriptor);
}

double TRANS2QUIK_CLIENT::TRANS2QUIK_ORDER_AWG_PRICE (Descriptor orderDescriptor)
{
    return Int_in_Empty_out(E_TRANS2QUIK_ORDER_AWG_PRICE, orderDescriptor);
}

#endif

LPTSTR TRANS2QUIK_CLIENT::Int_in_LPTSTR_out(func_codes fc, quint32 int_in)
{
    PrepareMessage<quint32>(data(), fc, int_in);
    WaitSendMessage();
    WaitReadMessage(fc);

    LPSTR res = ReadAndCreate_LPSTR_from_QString(_in);

    return (LPTSTR)res;
}


LPTSTR TRANS2QUIK_CLIENT::TRANS2QUIK_ORDER_USERID (Descriptor orderDescriptor)
{
    return Int_in_LPTSTR_out(E_TRANS2QUIK_ORDER_USERID, orderDescriptor);
}

LPTSTR TRANS2QUIK_CLIENT::TRANS2QUIK_ORDER_ACCOUNT (Descriptor orderDescriptor)
{
    return Int_in_LPTSTR_out(E_TRANS2QUIK_ORDER_ACCOUNT, orderDescriptor);
}

LPTSTR TRANS2QUIK_CLIENT::TRANS2QUIK_ORDER_BROKERREF (Descriptor orderDescriptor)
{
    return Int_in_LPTSTR_out(E_TRANS2QUIK_ORDER_BROKERREF, orderDescriptor);
}

LPTSTR TRANS2QUIK_CLIENT::TRANS2QUIK_ORDER_CLIENT_CODE (Descriptor orderDescriptor)
{
    return Int_in_LPTSTR_out(E_TRANS2QUIK_ORDER_CLIENT_CODE, orderDescriptor);
}

LPTSTR TRANS2QUIK_CLIENT::TRANS2QUIK_ORDER_FIRMID (Descriptor orderDescriptor)
{
    return Int_in_LPTSTR_out(E_TRANS2QUIK_ORDER_FIRMID, orderDescriptor);
}

#if defined(_WIN64) || defined(_64bit)
LPTSTR TRANS2QUIK_CLIENT::TRANS2QUIK_ORDER_REJECT_REASON (Descriptor orderDescriptor)
{
    return Int_in_LPTSTR_out(E_TRANS2QUIK_ORDER_REJECT_REASON, orderDescriptor);
}

#endif


long TRANS2QUIK_CLIENT::TRANS2QUIK_SUBSCRIBE_TRADES (LPSTR ClassCode, LPSTR Seccodes)
{
    return TwoLPSTR_in_Empty_out(E_TRANS2QUIK_SUBSCRIBE_TRADES, ClassCode, Seccodes);
}

long TRANS2QUIK_CLIENT::TRANS2QUIK_UNSUBSCRIBE_TRADES ()
{
    return Emty_in_Empty_out(E_TRANS2QUIK_UNSUBSCRIBE_TRADES);
}

long TRANS2QUIK_CLIENT::TRANS2QUIK_START_TRADES (TRANS2QUIK_TRADE_STATUS_CALLBACK pfnTradeStatusCallback)
{
    _pfnTradeStatusCallback = pfnTradeStatusCallback;
    return Int_in_Empty_out(E_TRANS2QUIK_UNSUBSCRIBE_TRADES, quint32(pfnTradeStatusCallback!=NULL));
}


long TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_DATE (Descriptor tradeDescriptor)
{
    return Int_in_Empty_out(E_TRANS2QUIK_TRADE_DATE, tradeDescriptor);
}

long TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_SETTLE_DATE (Descriptor tradeDescriptor)
{
    return Int_in_Empty_out(E_TRANS2QUIK_TRADE_SETTLE_DATE, tradeDescriptor);
}

long TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_TIME (Descriptor tradeDescriptor)
{
    return Int_in_Empty_out(E_TRANS2QUIK_TRADE_TIME, tradeDescriptor);
}

long TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_IS_MARGINAL (Descriptor tradeDescriptor)
{
    return Int_in_Empty_out(E_TRANS2QUIK_TRADE_IS_MARGINAL, tradeDescriptor);
}

double TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_ACCRUED_INT (Descriptor tradeDescriptor)
{
    return Int_in_Double_out(E_TRANS2QUIK_TRADE_ACCRUED_INT, tradeDescriptor);
}

double TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_YIELD (Descriptor tradeDescriptor)
{
    return Int_in_Double_out(E_TRANS2QUIK_TRADE_YIELD, tradeDescriptor);
}

double TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_TS_COMMISSION (Descriptor tradeDescriptor)
{
    return Int_in_Double_out(E_TRANS2QUIK_TRADE_TS_COMMISSION, tradeDescriptor);
}

double TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_CLEARING_CENTER_COMMISSION (Descriptor tradeDescriptor)
{
    return Int_in_Double_out(E_TRANS2QUIK_TRADE_CLEARING_CENTER_COMMISSION, tradeDescriptor);
}

double TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_EXCHANGE_COMMISSION (Descriptor tradeDescriptor)
{
    return Int_in_Double_out(E_TRANS2QUIK_TRADE_EXCHANGE_COMMISSION, tradeDescriptor);
}

double TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_TRADING_SYSTEM_COMMISSION (Descriptor tradeDescriptor)
{
    return Int_in_Double_out(E_TRANS2QUIK_TRADE_TRADING_SYSTEM_COMMISSION, tradeDescriptor);
}

double TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_PRICE2 (Descriptor tradeDescriptor)
{
    return Int_in_Double_out(E_TRANS2QUIK_TRADE_PRICE2, tradeDescriptor);
}

double TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_REPO_RATE (Descriptor tradeDescriptor)
{
    return Int_in_Double_out(E_TRANS2QUIK_TRADE_REPO_RATE, tradeDescriptor);
}

double TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_REPO_VALUE (Descriptor tradeDescriptor)
{
    return Int_in_Double_out(E_TRANS2QUIK_TRADE_REPO_VALUE, tradeDescriptor);
}

double TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_REPO2_VALUE (Descriptor tradeDescriptor)
{
    return Int_in_Double_out(E_TRANS2QUIK_TRADE_REPO2_VALUE, tradeDescriptor);
}

double TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_ACCRUED_INT2 (Descriptor tradeDescriptor)
{
    return Int_in_Double_out(E_TRANS2QUIK_TRADE_ACCRUED_INT2, tradeDescriptor);
}

long TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_REPO_TERM (Descriptor tradeDescriptor)
{
    return Int_in_Empty_out(E_TRANS2QUIK_TRADE_REPO_TERM, tradeDescriptor);
}

double TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_START_DISCOUNT (Descriptor tradeDescriptor)
{
    return Int_in_Double_out(E_TRANS2QUIK_TRADE_START_DISCOUNT, tradeDescriptor);
}

double TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_LOWER_DISCOUNT (Descriptor tradeDescriptor)
{
    return Int_in_Double_out(E_TRANS2QUIK_TRADE_LOWER_DISCOUNT, tradeDescriptor);
}

double TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_UPPER_DISCOUNT (Descriptor tradeDescriptor)
{
    return Int_in_Double_out(E_TRANS2QUIK_TRADE_UPPER_DISCOUNT, tradeDescriptor);
}

long TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_BLOCK_SECURITIES (Descriptor tradeDescriptor)
{
    return Int_in_Empty_out(E_TRANS2QUIK_TRADE_BLOCK_SECURITIES, tradeDescriptor);
}

long TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_PERIOD (Descriptor tradeDescriptor)
{
    return Int_in_Empty_out(E_TRANS2QUIK_TRADE_PERIOD, tradeDescriptor);
}

long TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_KIND (Descriptor tradeDescriptor)
{
    return Int_in_Empty_out(E_TRANS2QUIK_TRADE_KIND, tradeDescriptor);
}


FILETIME TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_FILETIME (Descriptor tradeDescriptor)
{    
    return Int_in_FILETIME_out(E_TRANS2QUIK_TRADE_FILETIME, tradeDescriptor);
}

long TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_DATE_TIME (Descriptor tradeDescriptor, long nTimeType)
{
    return TwoInt_in_Empty_out(E_TRANS2QUIK_TRADE_DATE_TIME, tradeDescriptor, nTimeType);
}

#if defined(_WIN64) || defined(_64bit)
double TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_BROKER_COMMISSION (Descriptor tradeDescriptor)
{
    return Int_in_Double_out(E_TRANS2QUIK_TRADE_BROKER_COMMISSION, tradeDescriptor);
}

long TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_TRANSID (Descriptor tradeDescriptor)
{
    return Int_in_Empty_out(E_TRANS2QUIK_TRADE_TRANSID, tradeDescriptor);
}

#endif

LPTSTR TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_CURRENCY (Descriptor tradeDescriptor)
{
    return Int_in_LPTSTR_out(E_TRANS2QUIK_TRADE_CURRENCY, tradeDescriptor);
}

LPTSTR TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_SETTLE_CURRENCY (Descriptor tradeDescriptor)
{
    return Int_in_LPTSTR_out(E_TRANS2QUIK_TRADE_SETTLE_CURRENCY, tradeDescriptor);
}

LPTSTR TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_SETTLE_CODE (Descriptor tradeDescriptor)
{
    return Int_in_LPTSTR_out(E_TRANS2QUIK_TRADE_SETTLE_CODE, tradeDescriptor);
}

LPTSTR TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_ACCOUNT (Descriptor tradeDescriptor)
{
    return Int_in_LPTSTR_out(E_TRANS2QUIK_TRADE_ACCOUNT, tradeDescriptor);
}

LPTSTR TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_BROKERREF (Descriptor tradeDescriptor)
{
    return Int_in_LPTSTR_out(E_TRANS2QUIK_TRADE_BROKERREF, tradeDescriptor);
}

LPTSTR TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_CLIENT_CODE (Descriptor tradeDescriptor)
{
    return Int_in_LPTSTR_out(E_TRANS2QUIK_TRADE_CLIENT_CODE, tradeDescriptor);
}

LPTSTR TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_USERID (Descriptor tradeDescriptor)
{
    return Int_in_LPTSTR_out(E_TRANS2QUIK_TRADE_USERID, tradeDescriptor);
}

LPTSTR TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_FIRMID (Descriptor tradeDescriptor)
{
    return Int_in_LPTSTR_out(E_TRANS2QUIK_TRADE_FIRMID, tradeDescriptor);
}

LPTSTR TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_PARTNER_FIRMID (Descriptor tradeDescriptor)
{
    return Int_in_LPTSTR_out(E_TRANS2QUIK_TRADE_PARTNER_FIRMID, tradeDescriptor);
}

LPTSTR TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_EXCHANGE_CODE (Descriptor tradeDescriptor)
{
    return Int_in_LPTSTR_out(E_TRANS2QUIK_TRADE_EXCHANGE_CODE, tradeDescriptor);
}

LPTSTR TRANS2QUIK_CLIENT::TRANS2QUIK_TRADE_STATION_ID (Descriptor tradeDescriptor)
{
    return Int_in_LPTSTR_out(E_TRANS2QUIK_TRADE_STATION_ID, tradeDescriptor);
}


#if defined(_WIN64) || defined(_64bit)
LPTSTR TRANS2QUIK_CLIENT::TRANS2QUIK_TRANSACTION_REPLY_CLASS_CODE (Descriptor tradeDescriptor)
{
    return Int_in_LPTSTR_out(E_TRANS2QUIK_TRANSACTION_REPLY_CLASS_CODE, tradeDescriptor);
}

LPTSTR TRANS2QUIK_CLIENT::TRANS2QUIK_TRANSACTION_REPLY_SEC_CODE (Descriptor tradeDescriptor)
{
    return Int_in_LPTSTR_out(E_TRANS2QUIK_TRANSACTION_REPLY_SEC_CODE, tradeDescriptor);
}

double TRANS2QUIK_CLIENT::TRANS2QUIK_TRANSACTION_REPLY_PRICE (Descriptor tradeDescriptor)
{
    return Int_in_Double_out(E_TRANS2QUIK_TRANSACTION_REPLY_PRICE, tradeDescriptor);
}

Quantity TRANS2QUIK_CLIENT::TRANS2QUIK_TRANSACTION_REPLY_QUANTITY (Descriptor tradeDescriptor)
{
    return (Quantity) Int_in_Empty_out(E_TRANS2QUIK_TRANSACTION_REPLY_QUANTITY, tradeDescriptor);
}

Quantity TRANS2QUIK_CLIENT::TRANS2QUIK_TRANSACTION_REPLY_BALANCE (Descriptor tradeDescriptor)
{
    return (Quantity) Int_in_Empty_out(E_TRANS2QUIK_TRANSACTION_REPLY_BALANCE, tradeDescriptor);
}

LPTSTR TRANS2QUIK_CLIENT::TRANS2QUIK_TRANSACTION_REPLY_FIRMID (Descriptor tradeDescriptor)
{
    return Int_in_LPTSTR_out(E_TRANS2QUIK_TRANSACTION_REPLY_FIRMID, tradeDescriptor);
}

LPTSTR TRANS2QUIK_CLIENT::TRANS2QUIK_TRANSACTION_REPLY_ACCOUNT (Descriptor tradeDescriptor)
{
    return Int_in_LPTSTR_out(E_TRANS2QUIK_TRANSACTION_REPLY_ACCOUNT, tradeDescriptor);
}

LPTSTR TRANS2QUIK_CLIENT::TRANS2QUIK_TRANSACTION_REPLY_CLIENT_CODE (Descriptor tradeDescriptor)
{
    return Int_in_LPTSTR_out(E_TRANS2QUIK_TRANSACTION_REPLY_CLIENT_CODE, tradeDescriptor);
}

LPTSTR TRANS2QUIK_CLIENT::TRANS2QUIK_TRANSACTION_REPLY_BROKERREF (Descriptor tradeDescriptor)
{
    return Int_in_LPTSTR_out(E_TRANS2QUIK_TRANSACTION_REPLY_BROKERREF, tradeDescriptor);
}

LPTSTR TRANS2QUIK_CLIENT::TRANS2QUIK_TRANSACTION_REPLY_EXCHANGE_CODE (Descriptor tradeDescriptor)
{
    return Int_in_LPTSTR_out(E_TRANS2QUIK_TRANSACTION_REPLY_EXCHANGE_CODE, tradeDescriptor);
}

#endif
