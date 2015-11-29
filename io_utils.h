#ifndef IO_UTILS_H
#define IO_UTILS_H

#include <QDataStream>
#include <QTcpSocket>
#include "func_codes.h"
#if !defined(WIN32) && !defined(WIN64)
#include "min_trans2quik_api_def.h"
#else
#include <trans2quik_api.h>
#endif

inline quint32 PrepareMessage(QDataStream& _data, func_codes fcode)
{
     _data << (quint32) sizeof(quint32);
     _data << quint32(fcode);
     return 2*sizeof(quint32);
}

inline void writeValue(QDataStream& /*data*/) {}
template<typename A, typename... Values>
void writeValue(QDataStream& data, const A& arg1, const Values&... args)
{
    data << arg1;
    writeValue(data, args...);
}

template<typename... Values>
quint32 PrepareMessage(QDataStream& data, func_codes fcode, Values... parameters)
{
#if defined(_WIN64) || defined(_64bit)
  quint64 old_pos = data.device()->pos();
#else
    quint32 old_pos = data.device()->pos();
#endif
  data << quint32(0);
  data << quint32(fcode);
  writeValue(data, parameters...);
  quint32 sz =(quint32) data.device()->size();
  data.device()->seek(old_pos);
  data << quint32(sz - sizeof(quint32));
  return sz;
}


inline void ReadQString2LPSTR(QDataStream& in, LPSTR dest)
{
    QString val;
    in >> val;
    std::string s = val.toStdString();
    quint32 sz = (quint32) s.size();
    if (sz && dest)
        memcpy(dest, s.data(), sz);
}

inline LPSTR ReadAndCreate_LPSTR_from_QString(QDataStream& in)
{
    QString val;
    in >> val;
    if (val.size() == 0) return NULL;
    std::string s = val.toStdString();
    quint32 sz = (quint32) s.size();
    LPSTR dest = NULL;
    if (sz > 0)
     dest = (LPSTR) malloc(sizeof(char)*sz+1);
    if (sz && dest)
    {
        memcpy(dest, s.data(), sz+1);
//        memset(dest+sz,0,1);
    }
    return dest;
}

template<class A, class B> void Read2Ptr(QDataStream& _in, B dest)
{
    A dummy;
    _in >> dummy;
    if (dest) *dest = dummy;
}

template<class A> const A Read2Val(QDataStream& _in)
{
    A dummy;
    _in >> dummy;
    return dummy;
}

#define ReadLongPtr Read2Ptr<quint32, long*>
#define ReadPDWORD Read2Ptr<quint32, PDWORD>
#define ReadEntityNumber Read2Ptr<EntityNumber, EntityNumber*>

#endif // IO_UTILS_H

