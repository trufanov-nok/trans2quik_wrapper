#ifndef MIN_TRANS2QUIK_API_H
#define MIN_TRANS2QUIK_API_H


typedef unsigned int DWORD;
typedef char* LPSTR;
typedef const char* LPCSTR;
typedef unsigned int* PDWORD;

#ifdef _UNICODE
typedef wchar_t TCHAR;
#else
typedef char TCHAR;
#endif

typedef TCHAR* LPTSTR;

typedef struct _FILETIME {
  DWORD dwLowDateTime;
  DWORD dwHighDateTime;
} FILETIME, *PFILETIME;

#if defined(_WIN64) || defined(_64bit)
typedef long long __int64;
typedef unsigned long long __uint64;
typedef __int64 Quantity;
typedef __uint64 EntityNumber;
typedef __int64 OrderDescriptor;
typedef __int64 TradeDescriptor;
typedef __int64 TransactionReplyDescriptor;

typedef void (*TRANS2QUIK_CONNECTION_STATUS_CALLBACK) (
                            long nConnectionEvent
                            , long nExtendedErrorCode
                            , LPCSTR lpcstrInfoMessage);

typedef void (*TRANS2QUIK_TRANSACTION_REPLY_CALLBACK) (
                            long nTransactionResult
                            , long nTransactionExtendedErrorCode
                            , long nTransactionReplyCode
                            , DWORD dwTransId
                            , EntityNumber nOrderNum
                            , LPCSTR lpcstrTransactionReplyMessage
                            , TransactionReplyDescriptor transReplyDescriptor);

typedef void (*TRANS2QUIK_ORDER_STATUS_CALLBACK) (
                            long nMode
                            , DWORD dwTransID
                            , EntityNumber dNumber
                            , LPCSTR ClassCode
                            , LPCSTR SecCode
                            , double dPrice
                            , Quantity  nBalance
                            , double dValue
                            , long nIsSell
                            , long nStatus
                            , OrderDescriptor orderDescriptor);

typedef void (*TRANS2QUIK_TRADE_STATUS_CALLBACK)      (
                            long nMode
                            , EntityNumber dNumber
                            , EntityNumber dOrderNumber
                            , LPCSTR ClassCode
                            , LPCSTR SecCode
                            , double dPrice
                            , Quantity nQty
                            , double dValue
                            , long nIsSell
                            , TradeDescriptor tradeDescriptor);

#else
typedef long Quantity;
typedef double EntityNumber;
typedef long OrderDescriptor;
typedef long TradeDescriptor;
typedef long TransactionReplyDescriptor;
typedef void (__stdcall *TRANS2QUIK_CONNECTION_STATUS_CALLBACK) (long nConnectionEvent, long nExtendedErrorCode, LPCSTR lpcstrInfoMessage);
typedef void (__stdcall *TRANS2QUIK_TRANSACTION_REPLY_CALLBACK) (long nTransactionResult, long nTransactionExtendedErrorCode, long nTransactionReplyCode, DWORD dwTransId, double dOrderNum, LPCSTR lpcstrTransactionReplyMessage);
typedef void (__stdcall *TRANS2QUIK_ORDER_STATUS_CALLBACK)      (long nMode, DWORD dwTransID, double dNumber, LPCSTR ClassCode, LPCSTR SecCode, double dPrice, long nBalance, double dValue, long nIsSell, long nStatus, long nOrderDescriptor);
typedef void (__stdcall *TRANS2QUIK_TRADE_STATUS_CALLBACK)      (long nMode, double dNumber, double dOrderNumber, LPCSTR ClassCode, LPCSTR SecCode, double dPrice, long nQty, double dValue, long nIsSell, long nTradeDescriptor);

#endif




#define TRANS2QUIK_SUCCESS						0
#define TRANS2QUIK_FAILED						1
#define TRANS2QUIK_QUIK_TERMINAL_NOT_FOUND		2
#define TRANS2QUIK_DLL_VERSION_NOT_SUPPORTED	3
#define TRANS2QUIK_ALREADY_CONNECTED_TO_QUIK	4
#define TRANS2QUIK_WRONG_SYNTAX					5
#define TRANS2QUIK_QUIK_NOT_CONNECTED			6
#define TRANS2QUIK_DLL_NOT_CONNECTED			7
#define TRANS2QUIK_QUIK_CONNECTED				8
#define TRANS2QUIK_QUIK_DISCONNECTED			9
#define TRANS2QUIK_DLL_CONNECTED				10
#define TRANS2QUIK_DLL_DISCONNECTED				11
#define TRANS2QUIK_MEMORY_ALLOCATION_ERROR		12
#define TRANS2QUIK_WRONG_CONNECTION_HANDLE		13
#define TRANS2QUIK_WRONG_INPUT_PARAMS			14

#define ORDER_QUIKDATE                          0
#define ORDER_QUIKTIME                          1  
#define ORDER_MICROSEC                          2
#define ORDER_WITHDRAW_QUIKDATE                 3
#define ORDER_WITHDRAW_QUIKTIME                 4
#define ORDER_WITHDRAW_MICROSEC                 5
#define TRADE_QUIKDATE                          0
#define TRADE_QUIKTIME                          1
#define TRADE_MICROSEC                          2

typedef OrderDescriptor Descriptor; // they all the same type

#endif
