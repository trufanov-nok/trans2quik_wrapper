#include "T2Q_Server.h"
#include "io_utils.h"
#include "trans2quik_api.h"

TRANS2QUIK_SERVER* TRANS2QUIK_SERVER::_srv = NULL;

TRANS2QUIK_SERVER::TRANS2QUIK_SERVER(): _buf(), _out(&_buf, QIODevice::WriteOnly)
{
    connect(this, SIGNAL(newConnection()), this, SLOT(clientConnected()));
    _out.setVersion(QDataStream::Qt_4_0);
    TRANS2QUIK_SERVER::_srv = this;
}

 QDataStream &TRANS2QUIK_SERVER::data()
{
    _buf.clear();
    _out.device()->seek(0);
    return _out;
}

void TRANS2QUIK_SERVER::clientConnected()
{
    curConnection = nextPendingConnection();

    connect(curConnection, SIGNAL(disconnected()), curConnection, SLOT(deleteLater()));
    connect(curConnection, SIGNAL(readyRead()), this, SLOT(processMessages()));
    QObject::connect(curConnection, SIGNAL(readyRead()), &_wait_loop, SLOT(quit()));
    _in.setDevice(curConnection);
    _in.setVersion(QDataStream::Qt_4_0);

}


void TRANS2QUIK_SERVER::SetCallback(func_codes& fc)
{
    quint32 is_callback_set;
    _in >> is_callback_set;

    switch ( fc )
    {
        case E_TRANS2QUIK_SET_CONNECTION_STATUS_CALLBACK:
        case E_TRANS2QUIK_SET_TRANSACTIONS_REPLY_CALLBACK:
    {
        quint32 dwErrorMessageSize;
        _in >> dwErrorMessageSize;
        LPSTR lpstErrorMessage = (dwErrorMessageSize>0)?(LPSTR)malloc(sizeof(char)*dwErrorMessageSize):nullptr;

        long ExtendedErrorCode = 0; long res;
        if (fc == E_TRANS2QUIK_SET_CONNECTION_STATUS_CALLBACK)
        {
         TRANS2QUIK_CONNECTION_STATUS_CALLBACK func = (is_callback_set)? &TRANS2QUIK_SERVER::_TRANS2QUIK_CONNECTION_STATUS_CALLBACK:NULL;
         res = TRANS2QUIK_SET_CONNECTION_STATUS_CALLBACK(func, &ExtendedErrorCode, lpstErrorMessage, dwErrorMessageSize);
        } else {
            TRANS2QUIK_TRANSACTION_REPLY_CALLBACK func = (is_callback_set)? &_TRANS2QUIK_TRANSACTION_REPLY_CALLBACK:NULL;
            res = TRANS2QUIK_SET_TRANSACTIONS_REPLY_CALLBACK(func, &ExtendedErrorCode, lpstErrorMessage, dwErrorMessageSize);
        }

        PrepareMessage<quint32, quint32, QString>(data(), E_TRANS2QUIK_SET_CONNECTION_STATUS_CALLBACK, res, ExtendedErrorCode, QString(lpstErrorMessage));
        curConnection->write(_buf);
        curConnection->flush();

        delete lpstErrorMessage;

    } break;

    case E_TRANS2QUIK_START_ORDERS:
    case E_TRANS2QUIK_START_TRADES:
    {
        long res;
                if (fc == E_TRANS2QUIK_START_ORDERS)
                {
                 TRANS2QUIK_ORDER_STATUS_CALLBACK func = (is_callback_set)? &_TRANS2QUIK_ORDER_STATUS_CALLBACK:NULL;
                 res = TRANS2QUIK_START_ORDERS(func);
                } else {
                    TRANS2QUIK_TRADE_STATUS_CALLBACK func = (is_callback_set)? &_TRANS2QUIK_TRADE_STATUS_CALLBACK:NULL;
                    res = TRANS2QUIK_START_TRADES(func);
                }

                PrepareMessage<quint32>(data(), E_TRANS2QUIK_SET_CONNECTION_STATUS_CALLBACK, res);
                curConnection->write(_buf);
                curConnection->flush();
    }
    default: break;
    }


}

void TRANS2QUIK_SERVER::Call_ExtError_ret_long(func_codes fc, T2Q_ExtErr_Long f)
{
    quint32 dwErrorMessageSize;
    _in >> dwErrorMessageSize;
    LPSTR lpstErrorMessage = (dwErrorMessageSize>0)?(LPSTR)malloc(sizeof(char)*dwErrorMessageSize):nullptr;

    long ExtendedErrorCode = 0;
    long res = f(&ExtendedErrorCode, lpstErrorMessage, dwErrorMessageSize);

    PrepareMessage<quint32, quint32, QString>(data(), fc, res, ExtendedErrorCode, QString(lpstErrorMessage));
    curConnection->write(_buf);
    curConnection->flush();

    delete lpstErrorMessage;
}

void TRANS2QUIK_SERVER::Call_TwoLPSTR_ret_long(func_codes fc, T2Q_TwoLPSTR_Long f)
{
    LPSTR arg1 = ReadAndCreate_LPSTR_from_QString(_in);
    LPSTR arg2 = ReadAndCreate_LPSTR_from_QString(_in);

    long res = f(arg1, arg2);

    PrepareMessage<quint32>(data(), fc, res);
    curConnection->write(_buf);
    curConnection->flush();

    delete arg1; delete arg2;
}

void TRANS2QUIK_SERVER::Call_Emty_ret_long(func_codes fc, T2Q_Empty_Long f)
{
    long res = f();
    PrepareMessage<quint32>(data(), fc, res);
    curConnection->write(_buf);
    curConnection->flush();
}

void TRANS2QUIK_SERVER::SendFILETIME(func_codes fc, FILETIME res)
{
    PrepareMessage<quint32, quint32>(data(), fc, res.dwHighDateTime, res.dwLowDateTime);
    curConnection->write(_buf);
    curConnection->flush();
}

void TRANS2QUIK_SERVER::SendLPTSTR(func_codes fc, LPTSTR res)
{
    PrepareMessage<QString>(data(), fc, res?QString::fromWCharArray(res):QString(""));
    curConnection->write(_buf);
    curConnection->flush();
}




void TRANS2QUIK_SERVER::processMessages()
{
    do
    {

        while (curConnection->bytesAvailable() < (int)sizeof(quint32)) {
            _wait_loop.exec();
        }

        quint32 dummy;
        _in >> dummy;

        while (curConnection->bytesAvailable() < dummy) {
            _wait_loop.exec();
        }

        _in >> dummy;
        func_codes func_code = (func_codes) dummy;
        switch ( func_code )
        {
        case E_TRANS2QUIK_SEND_SYNC_TRANSACTION:
        {  //long (LPSTR lpstTransactionString, long* pnReplyCode, PDWORD pdwTransId, EntityNumber* pnOrderNum, LPSTR lpstrResultMessage, DWORD dwResultMessageSize, long* pnExtendedErrorCode, LPSTR lpstErrorMessage, DWORD dwErrorMessageSize)
            LPSTR lpstTransactionString = ReadAndCreate_LPSTR_from_QString(_in);
            quint32 dwResultMessageSize;
            _in >> dwResultMessageSize;
            LPSTR lpstrResultMessage = (dwResultMessageSize>0)?(LPSTR)malloc(sizeof(char)*dwResultMessageSize):nullptr;
            quint32 dwErrorMessageSize;
            _in >> dwErrorMessageSize;
            LPSTR lpstErrorMessage = (dwErrorMessageSize>0)?(LPSTR)malloc(sizeof(char)*dwErrorMessageSize):nullptr;

            long ReplyCode = 0;
            DWORD dwTransId = 0;
            EntityNumber OrderNum = 0;
            long ExtendedErrorCode = 0;

            long res = TRANS2QUIK_SEND_SYNC_TRANSACTION(lpstTransactionString, &ReplyCode, (PDWORD) &dwTransId, &OrderNum, lpstrResultMessage, dwResultMessageSize, &ExtendedErrorCode, lpstErrorMessage, dwErrorMessageSize);

            //            ReadLongPtr(_in, pnReplyCode);
            //            ReadPDWORD(_in, pdwTransId);
            //            ReadEntityNumber(_in, pnOrderNum);
            //            ReadQString2LPSTR(_in, lpstrResultMessage);
            //            ReadLongPtr(_in, pnExtendedErrorCode);
            //            ReadQString2LPSTR(_in, lpstErrorMessage);

            PrepareMessage<quint32, quint32, quint32, quint32, QString, quint32, QString>
                    (data(), E_TRANS2QUIK_SEND_SYNC_TRANSACTION, res,
                     ReplyCode, dwTransId, OrderNum, lpstrResultMessage?QString(lpstrResultMessage):QString(),
                     ExtendedErrorCode, lpstErrorMessage?QString(lpstErrorMessage):QString());

            curConnection->write(_buf);
            curConnection->flush();
            delete lpstTransactionString;
            delete lpstrResultMessage;
            delete lpstErrorMessage;
        } break;

        case E_TRANS2QUIK_SEND_ASYNC_TRANSACTION:
        case E_TRANS2QUIK_CONNECT:
        {
            LPSTR lpstTransactionString = ReadAndCreate_LPSTR_from_QString(_in);
            quint32 dwErrorMessageSize;
            _in >> dwErrorMessageSize;
            LPSTR lpstErrorMessage = (dwErrorMessageSize>0)?(LPSTR)malloc(sizeof(char)*dwErrorMessageSize):nullptr;

            long ExtendedErrorCode = 0;
            long res = 0;
            if (func_code == E_TRANS2QUIK_SEND_ASYNC_TRANSACTION)
            res = TRANS2QUIK_SEND_ASYNC_TRANSACTION(lpstTransactionString, &ExtendedErrorCode, lpstErrorMessage, dwErrorMessageSize);
            else
                res = TRANS2QUIK_CONNECT(lpstTransactionString, &ExtendedErrorCode, lpstErrorMessage, dwErrorMessageSize);

            PrepareMessage<quint32, quint32, QString>(data(), func_code, res, ExtendedErrorCode, QString(lpstErrorMessage));
            curConnection->write(_buf);
            curConnection->flush();

            delete lpstErrorMessage;
            delete lpstTransactionString;

        } break;
        case E_TRANS2QUIK_DISCONNECT:
        {
            Call_ExtError_ret_long(E_TRANS2QUIK_DISCONNECT, TRANS2QUIK_DISCONNECT);
        } break;

        case E_TRANS2QUIK_SET_CONNECTION_STATUS_CALLBACK: //long (TRANS2QUIK_CONNECTION_STATUS_CALLBACK pfConnectionStatusCallback, long* pnExtendedErrorCode, LPSTR lpstrErrorMessage, DWORD dwErrorMessageSize)
        case E_TRANS2QUIK_SET_TRANSACTIONS_REPLY_CALLBACK: //long (TRANS2QUIK_TRANSACTION_REPLY_CALLBACK pfTransactionReplyCallback, long* pnExtendedErrorCode, LPSTR lpstrErrorMessage, DWORD dwErrorMessageSize)
        case E_TRANS2QUIK_START_ORDERS: //long (TRANS2QUIK_ORDER_STATUS_CALLBACK pfnOrderStatusCallback)
        case E_TRANS2QUIK_START_TRADES: //long (TRANS2QUIK_TRADE_STATUS_CALLBACK pfnTradeStatusCallback)
        {
          SetCallback(func_code);
        }
        break;

        case E_TRANS2QUIK_IS_QUIK_CONNECTED: //long (long* pnExtendedErrorCode, LPSTR lpstrErrorMessage, DWORD dwErrorMessageSize)
        {
            Call_ExtError_ret_long(E_TRANS2QUIK_IS_QUIK_CONNECTED, TRANS2QUIK_IS_QUIK_CONNECTED);
        }
        break;

        case E_TRANS2QUIK_IS_DLL_CONNECTED: //long (long* pnExtendedErrorCode, LPSTR lpstrErrorMessage, DWORD dwErrorMessageSize)
        {
            Call_ExtError_ret_long(E_TRANS2QUIK_IS_DLL_CONNECTED, TRANS2QUIK_IS_DLL_CONNECTED);
        }
        break;


        case E_TRANS2QUIK_SUBSCRIBE_ORDERS: //long (LPSTR ClassCode, LPSTR Seccodes)
        {
            Call_TwoLPSTR_ret_long(E_TRANS2QUIK_SUBSCRIBE_ORDERS, TRANS2QUIK_SUBSCRIBE_ORDERS);
        }
        break;

        case E_TRANS2QUIK_UNSUBSCRIBE_ORDERS: //long ()
        {

            Call_Emty_ret_long(E_TRANS2QUIK_UNSUBSCRIBE_ORDERS, TRANS2QUIK_UNSUBSCRIBE_ORDERS);
        }
        break;

        case E_TRANS2QUIK_ORDER_QTY: //Quantity (OrderDescriptor orderDescriptor)
        {
            SendRes<quint32>(E_TRANS2QUIK_ORDER_QTY, TRANS2QUIK_ORDER_QTY(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_ORDER_DATE: //long (OrderDescriptor orderDescriptor)
        {
            SendRes<quint32>(E_TRANS2QUIK_ORDER_DATE, TRANS2QUIK_ORDER_DATE(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_ORDER_TIME: //long (OrderDescriptor orderDescriptor)
        {
            SendRes<quint32>(E_TRANS2QUIK_ORDER_TIME, TRANS2QUIK_ORDER_TIME(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_ORDER_ACTIVATION_TIME: //long (OrderDescriptor orderDescriptor)
        {
            SendRes<quint32>(E_TRANS2QUIK_ORDER_ACTIVATION_TIME, TRANS2QUIK_ORDER_ACTIVATION_TIME(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_ORDER_WITHDRAW_TIME: //long (OrderDescriptor orderDescriptor)
        {
            SendRes<quint32>(E_TRANS2QUIK_ORDER_WITHDRAW_TIME, TRANS2QUIK_ORDER_WITHDRAW_TIME(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_ORDER_EXPIRY: //long (OrderDescriptor orderDescriptor)
        {
            SendRes<quint32>(E_TRANS2QUIK_ORDER_EXPIRY, TRANS2QUIK_ORDER_EXPIRY(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_ORDER_ACCRUED_INT: //double (OrderDescriptor orderDescriptor)
        {
            SendRes<qreal>(E_TRANS2QUIK_ORDER_ACCRUED_INT, TRANS2QUIK_ORDER_ACCRUED_INT(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_ORDER_YIELD: //double (OrderDescriptor orderDescriptor)
        {
            SendRes<qreal>(E_TRANS2QUIK_ORDER_YIELD, TRANS2QUIK_ORDER_YIELD(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_ORDER_UID: //long (OrderDescriptor orderDescriptor)
        {
            SendRes<quint32>(E_TRANS2QUIK_ORDER_UID, TRANS2QUIK_ORDER_UID(Read2Val<Descriptor>(_in)));
        }
        break;


        case E_TRANS2QUIK_ORDER_VISIBLE_QTY: //Quantity (OrderDescriptor orderDescriptor)
        {
            SendRes<quint32>(E_TRANS2QUIK_ORDER_VISIBLE_QTY, TRANS2QUIK_ORDER_VISIBLE_QTY(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_ORDER_PERIOD: //long (OrderDescriptor orderDescriptor)
        {
            SendRes<quint32>(E_TRANS2QUIK_ORDER_PERIOD, TRANS2QUIK_ORDER_PERIOD(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_ORDER_FILETIME: //FILETIME (OrderDescriptor orderDescriptor)
        {
             SendFILETIME(E_TRANS2QUIK_ORDER_FILETIME, TRANS2QUIK_ORDER_FILETIME(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_ORDER_DATE_TIME: //long (OrderDescriptor orderDescriptor, long nTimeType)
        {
            Descriptor orderDescriptor;
            _in >> orderDescriptor;
            qint32 nTimeType;
            _in >> nTimeType;
            SendRes<quint32>(E_TRANS2QUIK_ORDER_DATE_TIME, TRANS2QUIK_ORDER_DATE_TIME(orderDescriptor, nTimeType));
        }
        break;

        case E_TRANS2QUIK_ORDER_WITHDRAW_FILETIME: //FILETIME (OrderDescriptor orderDescriptor)
        {
            SendFILETIME(E_TRANS2QUIK_ORDER_WITHDRAW_FILETIME, TRANS2QUIK_ORDER_WITHDRAW_FILETIME(Read2Val<Descriptor>(_in)));
        }
        break;


        case E_TRANS2QUIK_ORDER_VALUE_ENTRY_TYPE: //long (OrderDescriptor orderDescriptor)
        {
            SendRes<quint32>(E_TRANS2QUIK_ORDER_VALUE_ENTRY_TYPE, TRANS2QUIK_ORDER_VALUE_ENTRY_TYPE(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_ORDER_EXTENDED_FLAGS: //long (OrderDescriptor orderDescriptor)
        {
            SendRes<quint32>(E_TRANS2QUIK_ORDER_EXTENDED_FLAGS, TRANS2QUIK_ORDER_EXTENDED_FLAGS(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_ORDER_MIN_QTY: //Quantity (OrderDescriptor orderDescriptor)
        {
            SendRes<quint32>(E_TRANS2QUIK_ORDER_MIN_QTY, TRANS2QUIK_ORDER_MIN_QTY(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_ORDER_EXEC_TYPE: //long (OrderDescriptor orderDescriptor)
        {
            SendRes<quint32>(E_TRANS2QUIK_ORDER_EXEC_TYPE, TRANS2QUIK_ORDER_EXEC_TYPE(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_ORDER_AWG_PRICE: //double (OrderDescriptor orderDescriptor)
        {
            SendRes<qreal>(E_TRANS2QUIK_ORDER_AWG_PRICE, TRANS2QUIK_ORDER_AWG_PRICE(Read2Val<Descriptor>(_in)));
        }
        break;


        case E_TRANS2QUIK_ORDER_USERID: //LPTSTR (OrderDescriptor orderDescriptor)
        {
            SendLPTSTR(E_TRANS2QUIK_ORDER_USERID, TRANS2QUIK_ORDER_USERID(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_ORDER_ACCOUNT: //LPTSTR (OrderDescriptor orderDescriptor)
        {
            SendLPTSTR(E_TRANS2QUIK_ORDER_ACCOUNT, TRANS2QUIK_ORDER_ACCOUNT(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_ORDER_BROKERREF: //LPTSTR (OrderDescriptor orderDescriptor)
        {
            SendLPTSTR(E_TRANS2QUIK_ORDER_BROKERREF, TRANS2QUIK_ORDER_BROKERREF(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_ORDER_CLIENT_CODE: //LPTSTR (OrderDescriptor orderDescriptor)
        {
            SendLPTSTR(E_TRANS2QUIK_ORDER_CLIENT_CODE, TRANS2QUIK_ORDER_CLIENT_CODE(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_ORDER_FIRMID: //LPTSTR (OrderDescriptor orderDescriptor)
        {
            SendLPTSTR(E_TRANS2QUIK_ORDER_FIRMID, TRANS2QUIK_ORDER_FIRMID(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_ORDER_REJECT_REASON: //LPTSTR (OrderDescriptor orderDescriptor)
        {
            SendLPTSTR(E_TRANS2QUIK_ORDER_REJECT_REASON, TRANS2QUIK_ORDER_REJECT_REASON(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_SUBSCRIBE_TRADES: //long (LPSTR ClassCode, LPSTR Seccodes)
        {
            Call_TwoLPSTR_ret_long(E_TRANS2QUIK_SUBSCRIBE_TRADES, TRANS2QUIK_SUBSCRIBE_TRADES);
        }
        break;

        case E_TRANS2QUIK_UNSUBSCRIBE_TRADES: //long ()
        {
            Call_Emty_ret_long(E_TRANS2QUIK_UNSUBSCRIBE_TRADES, TRANS2QUIK_UNSUBSCRIBE_TRADES);
        }
        break;

        case E_TRANS2QUIK_TRADE_DATE: //long (TradeDescriptor tradeDescriptor)
        {
            SendRes<quint32>(E_TRANS2QUIK_TRADE_DATE, TRANS2QUIK_TRADE_DATE(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRADE_SETTLE_DATE: //long (TradeDescriptor tradeDescriptor)
        {
            SendRes<quint32>(E_TRANS2QUIK_TRADE_SETTLE_DATE, TRANS2QUIK_TRADE_SETTLE_DATE(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRADE_TIME: //long (TradeDescriptor tradeDescriptor)
        {
            SendRes<quint32>(E_TRANS2QUIK_TRADE_TIME, TRANS2QUIK_TRADE_TIME(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRADE_IS_MARGINAL: //long (TradeDescriptor tradeDescriptor)
        {
            SendRes<quint32>(E_TRANS2QUIK_TRADE_IS_MARGINAL, TRANS2QUIK_TRADE_IS_MARGINAL(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRADE_ACCRUED_INT: //double (TradeDescriptor tradeDescriptor)
        {
            SendRes<double>(E_TRANS2QUIK_TRADE_ACCRUED_INT, TRANS2QUIK_TRADE_ACCRUED_INT(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRADE_YIELD: //double (TradeDescriptor tradeDescriptor)
        {
            SendRes<double>(E_TRANS2QUIK_TRADE_YIELD, TRANS2QUIK_TRADE_YIELD(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRADE_TS_COMMISSION: //double (TradeDescriptor tradeDescriptor)
        {
            SendRes<double>(E_TRANS2QUIK_TRADE_TS_COMMISSION, TRANS2QUIK_TRADE_TS_COMMISSION(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRADE_CLEARING_CENTER_COMMISSION: //double (TradeDescriptor tradeDescriptor)
        {
            SendRes<double>(E_TRANS2QUIK_TRADE_CLEARING_CENTER_COMMISSION, TRANS2QUIK_TRADE_CLEARING_CENTER_COMMISSION(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRADE_EXCHANGE_COMMISSION: //double (TradeDescriptor tradeDescriptor)
        {
            SendRes<double>(E_TRANS2QUIK_TRADE_EXCHANGE_COMMISSION, TRANS2QUIK_TRADE_EXCHANGE_COMMISSION(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRADE_TRADING_SYSTEM_COMMISSION: //double (TradeDescriptor tradeDescriptor)
        {
            SendRes<double>(E_TRANS2QUIK_TRADE_TRADING_SYSTEM_COMMISSION, TRANS2QUIK_TRADE_TRADING_SYSTEM_COMMISSION(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRADE_PRICE2: //double (TradeDescriptor tradeDescriptor)
        {
            SendRes<double>(E_TRANS2QUIK_TRADE_PRICE2, TRANS2QUIK_TRADE_PRICE2(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRADE_REPO_RATE: //double (TradeDescriptor tradeDescriptor)
        {
            SendRes<double>(E_TRANS2QUIK_TRADE_REPO_RATE, TRANS2QUIK_TRADE_REPO_RATE(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRADE_REPO_VALUE: //double (TradeDescriptor tradeDescriptor)
        {
            SendRes<double>(E_TRANS2QUIK_TRADE_REPO_VALUE, TRANS2QUIK_TRADE_REPO_VALUE(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRADE_REPO2_VALUE: //double (TradeDescriptor tradeDescriptor)
        {
            SendRes<double>(E_TRANS2QUIK_TRADE_REPO2_VALUE, TRANS2QUIK_TRADE_REPO2_VALUE(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRADE_ACCRUED_INT2: //double (TradeDescriptor tradeDescriptor)
        {
            SendRes<double>(E_TRANS2QUIK_TRADE_ACCRUED_INT2, TRANS2QUIK_TRADE_ACCRUED_INT2(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRADE_REPO_TERM: //long (TradeDescriptor tradeDescriptor)
        {
            SendRes<quint32>(E_TRANS2QUIK_TRADE_REPO_TERM, TRANS2QUIK_TRADE_REPO_TERM(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRADE_START_DISCOUNT: //double (TradeDescriptor tradeDescriptor)
        {
            SendRes<double>(E_TRANS2QUIK_TRADE_START_DISCOUNT, TRANS2QUIK_TRADE_START_DISCOUNT(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRADE_LOWER_DISCOUNT: //double (TradeDescriptor tradeDescriptor)
        {
            SendRes<double>(E_TRANS2QUIK_TRADE_LOWER_DISCOUNT, TRANS2QUIK_TRADE_LOWER_DISCOUNT(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRADE_UPPER_DISCOUNT: //double (TradeDescriptor tradeDescriptor)
        {
            SendRes<double>(E_TRANS2QUIK_TRADE_UPPER_DISCOUNT, TRANS2QUIK_TRADE_UPPER_DISCOUNT(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRADE_BLOCK_SECURITIES: //long (TradeDescriptor tradeDescriptor)
        {
            SendRes<quint32>(E_TRANS2QUIK_TRADE_BLOCK_SECURITIES, TRANS2QUIK_TRADE_BLOCK_SECURITIES(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRADE_PERIOD: //long (TradeDescriptor tradeDescriptor)
        {
            SendRes<quint32>(E_TRANS2QUIK_TRADE_PERIOD, TRANS2QUIK_TRADE_PERIOD(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRADE_KIND: //long (TradeDescriptor tradeDescriptor)
        {
            SendRes<quint32>(E_TRANS2QUIK_TRADE_ACCRUED_INT2, TRANS2QUIK_TRADE_ACCRUED_INT2(Read2Val<Descriptor>(_in)));
        }
        break;


        case E_TRANS2QUIK_TRADE_FILETIME: // FILETIME (TradeDescriptor tradeDescriptor)
        {
            SendFILETIME(E_TRANS2QUIK_TRADE_FILETIME, TRANS2QUIK_TRADE_FILETIME(Read2Val<Descriptor>(_in)));
        } break;

        case E_TRANS2QUIK_TRADE_DATE_TIME: // long (TradeDescriptor tradeDescriptor, long nTimeType)
        {
            Descriptor tradeDescriptor;
            _in >> tradeDescriptor;
            qint32 nTimeType;
            _in >> nTimeType;
            SendRes<quint32>(E_TRANS2QUIK_TRADE_DATE_TIME, TRANS2QUIK_TRADE_DATE_TIME(tradeDescriptor, nTimeType));
        } break;

        case E_TRANS2QUIK_TRADE_BROKER_COMMISSION: //double (TradeDescriptor tradeDescriptor)
        {
            SendRes<double>(E_TRANS2QUIK_TRADE_BROKER_COMMISSION, TRANS2QUIK_TRADE_BROKER_COMMISSION(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRADE_TRANSID: //long (TradeDescriptor tradeDescriptor)
        {
            SendRes<quint32>(E_TRANS2QUIK_TRADE_TRANSID, TRANS2QUIK_TRADE_TRANSID(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRADE_CURRENCY: //LPTSTR (TradeDescriptor tradeDescriptor)
        {
            SendLPTSTR(E_TRANS2QUIK_TRADE_CURRENCY, TRANS2QUIK_TRADE_CURRENCY(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRADE_SETTLE_CURRENCY: // LPTSTR (TradeDescriptor tradeDescriptor)
        {
             SendLPTSTR(E_TRANS2QUIK_TRADE_CURRENCY, TRANS2QUIK_TRADE_SETTLE_CURRENCY(Read2Val<Descriptor>(_in)));
        } break;

        case E_TRANS2QUIK_TRADE_SETTLE_CODE: //LPTSTR (TradeDescriptor tradeDescriptor)
        {
             SendLPTSTR(E_TRANS2QUIK_TRADE_SETTLE_CODE, TRANS2QUIK_TRADE_SETTLE_CODE(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRADE_ACCOUNT: //LPTSTR (TradeDescriptor tradeDescriptor)
        {
             SendLPTSTR(E_TRANS2QUIK_TRADE_ACCOUNT, TRANS2QUIK_TRADE_ACCOUNT(Read2Val<Descriptor>(_in)));
        }
        break;
        case E_TRANS2QUIK_TRADE_BROKERREF: //LPTSTR (TradeDescriptor tradeDescriptor)
        {
            SendLPTSTR(E_TRANS2QUIK_TRADE_BROKERREF, TRANS2QUIK_TRADE_BROKERREF(Read2Val<Descriptor>(_in)));
        }
        break;
        case E_TRANS2QUIK_TRADE_CLIENT_CODE: //LPTSTR (TradeDescriptor tradeDescriptor)
        {
            SendLPTSTR(E_TRANS2QUIK_TRADE_CLIENT_CODE, TRANS2QUIK_TRADE_CLIENT_CODE(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRADE_USERID: //LPTSTR (TradeDescriptor tradeDescriptor)
        {
            SendLPTSTR(E_TRANS2QUIK_TRADE_USERID, TRANS2QUIK_TRADE_USERID(Read2Val<Descriptor>(_in)));
        }
        break;
        case E_TRANS2QUIK_TRADE_FIRMID: //LPTSTR (TradeDescriptor tradeDescriptor)
        {
            SendLPTSTR(E_TRANS2QUIK_TRADE_FIRMID, TRANS2QUIK_TRADE_FIRMID(Read2Val<Descriptor>(_in)));
        }
        break;
        case E_TRANS2QUIK_TRADE_PARTNER_FIRMID: //LPTSTR (TradeDescriptor tradeDescriptor)
        {
            SendLPTSTR(E_TRANS2QUIK_TRADE_PARTNER_FIRMID, TRANS2QUIK_TRADE_PARTNER_FIRMID(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRADE_EXCHANGE_CODE: //LPTSTR (TradeDescriptor tradeDescriptor)
        {
            SendLPTSTR(E_TRANS2QUIK_TRADE_EXCHANGE_CODE, TRANS2QUIK_TRADE_EXCHANGE_CODE(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRADE_STATION_ID: //LPTSTR (TradeDescriptor tradeDescriptor)
        {
            SendLPTSTR(E_TRANS2QUIK_TRADE_STATION_ID, TRANS2QUIK_TRADE_STATION_ID(Read2Val<Descriptor>(_in)));
        }
        break;
        ////////////////////////////////////////////////////////////////////////
        case E_TRANS2QUIK_TRANSACTION_REPLY_CLASS_CODE: //LPTSTR (TransactionReplyDescriptor tradeDescriptor)
        {
            SendLPTSTR(E_TRANS2QUIK_TRANSACTION_REPLY_CLASS_CODE, TRANS2QUIK_TRANSACTION_REPLY_CLASS_CODE(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRANSACTION_REPLY_SEC_CODE: //LPTSTR (TransactionReplyDescriptor tradeDescriptor)
        {
            SendLPTSTR(E_TRANS2QUIK_TRANSACTION_REPLY_SEC_CODE, TRANS2QUIK_TRANSACTION_REPLY_SEC_CODE(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRANSACTION_REPLY_PRICE: //double (TransactionReplyDescriptor tradeDescriptor)
        {
             SendRes<double>(E_TRANS2QUIK_TRANSACTION_REPLY_PRICE, TRANS2QUIK_TRANSACTION_REPLY_PRICE(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRANSACTION_REPLY_QUANTITY: //Quantity (TransactionReplyDescriptor tradeDescriptor)
        {
            SendRes<Quantity>(E_TRANS2QUIK_TRANSACTION_REPLY_QUANTITY, TRANS2QUIK_TRANSACTION_REPLY_QUANTITY(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRANSACTION_REPLY_BALANCE: //Quantity (TransactionReplyDescriptor tradeDescriptor)
        {
            SendRes<Quantity>(E_TRANS2QUIK_TRANSACTION_REPLY_BALANCE, TRANS2QUIK_TRANSACTION_REPLY_BALANCE(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRANSACTION_REPLY_FIRMID: //LPTSTR (TransactionReplyDescriptor tradeDescriptor)
        {
            SendLPTSTR(E_TRANS2QUIK_TRANSACTION_REPLY_FIRMID, TRANS2QUIK_TRANSACTION_REPLY_FIRMID(Read2Val<Descriptor>(_in)));
        }
        break;
        case E_TRANS2QUIK_TRANSACTION_REPLY_ACCOUNT: //LPTSTR (TransactionReplyDescriptor tradeDescriptor)
        {
            SendLPTSTR(E_TRANS2QUIK_TRANSACTION_REPLY_ACCOUNT, TRANS2QUIK_TRANSACTION_REPLY_ACCOUNT(Read2Val<Descriptor>(_in)));
        }
        break;
        case E_TRANS2QUIK_TRANSACTION_REPLY_CLIENT_CODE: //LPTSTR (TransactionReplyDescriptor tradeDescriptor)
        {
            SendLPTSTR(E_TRANS2QUIK_TRANSACTION_REPLY_CLIENT_CODE, TRANS2QUIK_TRANSACTION_REPLY_CLIENT_CODE(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRANSACTION_REPLY_BROKERREF: //LPTSTR (TransactionReplyDescriptor tradeDescriptor)
        {
            SendLPTSTR(E_TRANS2QUIK_TRANSACTION_REPLY_BROKERREF, TRANS2QUIK_TRANSACTION_REPLY_BROKERREF(Read2Val<Descriptor>(_in)));
        }
        break;

        case E_TRANS2QUIK_TRANSACTION_REPLY_EXCHANGE_CODE: //LPTSTR (TransactionReplyDescriptor tradeDescriptor)
        {
            SendLPTSTR(E_TRANS2QUIK_TRANSACTION_REPLY_EXCHANGE_CODE, TRANS2QUIK_TRANSACTION_REPLY_EXCHANGE_CODE(Read2Val<Descriptor>(_in)));
        }
        break;


        default: break;
        }

    } while (curConnection->bytesAvailable() != 0);

}

void TRANS2QUIK_SERVER::_TRANS2QUIK_CONNECTION_STATUS_CALLBACK (long nConnectionEvent, long nExtendedErrorCode, LPCSTR lpcstrInfoMessage)
{    
    PrepareMessage<quint32, quint32, QString>
            (_srv->data(), E_TRANS2QUIK_CONNECTION_STATUS_CALLBACK, nConnectionEvent,
             nExtendedErrorCode, lpcstrInfoMessage?QString(lpcstrInfoMessage):QString());

    if (!_srv->curConnection.isNull())
    {
    _srv->curConnection->write(_srv->_buf);
    _srv->curConnection->flush();
    }
}

#if defined(_WIN64) || defined(_64bit)
void TRANS2QUIK_SERVER::_TRANS2QUIK_TRANSACTION_REPLY_CALLBACK (long nTransactionResult, long nTransactionExtendedErrorCode, long nTransactionReplyCode, DWORD dwTransId, EntityNumber OrderNum, PCSTR lpcstrTransactionReplyMessage, TransactionReplyDescriptor transReplyDescriptor)
{
    PrepareMessage<quint32, quint32, quint32, quint32, double, QString, TransactionReplyDescriptor>
            (_srv->data(), E_TRANS2QUIK_TRANSACTION_REPLY_CALLBACK, nTransactionResult, nTransactionExtendedErrorCode,
             nTransactionReplyCode, dwTransId, OrderNum, lpcstrTransactionReplyMessage?QString(lpcstrTransactionReplyMessage):QString(), transReplyDescriptor);
#else
void  TRANS2QUIK_SERVER::_TRANS2QUIK_TRANSACTION_REPLY_CALLBACK (long nTransactionResult, long nTransactionExtendedErrorCode, long nTransactionReplyCode, DWORD dwTransId, EntityNumber OrderNum, LPCSTR lpcstrTransactionReplyMessage)
{
    TRANS2QUIK_SERVER* srv = TRANS2QUIK_SERVER::srv;
    PrepareMessage<quint32, quint32, quint32, quint32, double, QString>
            (srv->data(), E_TRANS2QUIK_TRANSACTION_REPLY_CALLBACK, nTransactionResult, nTransactionExtendedErrorCode,
             nTransactionReplyCode, dwTransId, OrderNum, lpcstrTransactionReplyMessage?QString(lpcstrTransactionReplyMessage):QString());
#endif
    if (!_srv->curConnection.isNull())
    {
    _srv->curConnection->write(_srv->_buf);
    _srv->curConnection->flush();
    }
}

#if defined(_WIN64) || defined(_64bit)
void TRANS2QUIK_SERVER::_TRANS2QUIK_ORDER_STATUS_CALLBACK      (long nMode, DWORD dwTransID, EntityNumber dNumber, LPCSTR ClassCode, LPCSTR SecCode, double dPrice, Quantity  nBalance, double dValue, long nIsSell, long nStatus, OrderDescriptor orderDescriptor)
#else
void  TRANS2QUIK_SERVER::_TRANS2QUIK_ORDER_STATUS_CALLBACK      (long nMode, DWORD dwTransID, double dNumber, LPCSTR ClassCode, LPCSTR SecCode, double dPrice, long nBalance, double dValue, long nIsSell, long nStatus, long orderDescriptor)
#endif
{        
    PrepareMessage<quint32, quint32, double, QString, QString, double, quint32,  double, quint32, quint32, quint32 >
            (_srv->data(), E_TRANS2QUIK_ORDER_STATUS_CALLBACK, nMode, dwTransID, dNumber, ClassCode?QString(ClassCode):QString(), SecCode?QString(SecCode):QString(),
              dPrice, nBalance, dValue, nIsSell, nStatus, orderDescriptor);

    if (!_srv->curConnection.isNull())
    {
    _srv->curConnection->write(_srv->_buf);
    _srv->curConnection->flush();
    }
}

#if defined(_WIN64) || defined(_64bit)
void TRANS2QUIK_SERVER::_TRANS2QUIK_TRADE_STATUS_CALLBACK      (long nMode, EntityNumber dNumber, EntityNumber dOrderNumber, LPCSTR ClassCode, LPCSTR SecCode, double dPrice, Quantity nQty, double dValue, long nIsSell, TradeDescriptor tradeDescriptor)
#else
void TRANS2QUIK_SERVER::_TRANS2QUIK_TRADE_STATUS_CALLBACK      (long nMode, double dNumber, double dOrderNumber, LPCSTR ClassCode, LPCSTR SecCode, double dPrice, long nQty, double dValue, long nIsSell, long tradeDescriptor)
#endif
{
    PrepareMessage<quint32, double, double, QString, QString, double, quint32,  double, quint32, quint32 >
            (_srv->data(), E_TRANS2QUIK_TRADE_STATUS_CALLBACK, nMode, dNumber, dOrderNumber, ClassCode?QString(ClassCode):QString(), SecCode?QString(SecCode):QString(),
              dPrice, nQty, dValue, nIsSell, tradeDescriptor);

    if (!_srv->curConnection.isNull())
    {
    _srv->curConnection->write(_srv->_buf);
    _srv->curConnection->flush();
    }
}
