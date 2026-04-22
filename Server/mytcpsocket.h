#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include "msghandler.h"
#include "protocol.h"
#include <QObject>
#include <QTcpSocket>

class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    MyTcpSocket();
    ~MyTcpSocket();
    MsgHandler* m_pmh;
    QString m_strLoginName;
    QByteArray buffer;
    void sendMsg(PDU* pdu);
    PDU* readMsg();
    PDU* handleMsg(PDU* pdu);

public slots:
    //接收信息
    void recvMsg();
    void clientOffline();
};

#endif // MYTCPSOCKET_H
