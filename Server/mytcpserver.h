#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QThreadPool>
#include "mytcpsocket.h"

class MyTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    QThreadPool threadPool;

    static MyTcpServer& getInstance();
    //在listen监听的时候，与客户端连接成功，系统会自动运行incomingConnection
    //但是这个函数在QTcpServer中，必须要重写的一个函数
    //所以定义MyTcpServer来继承父类QTcpServer对该函数进行重写
    //参数qintptr handle是连接成功的客户端
    void incomingConnection(qintptr handle) override;
    void RemoveSocket(MyTcpSocket* mysocket);
    void resend(char *tarName, PDU* pdu);
private:
    MyTcpServer();
    MyTcpServer(const MyTcpServer& instance) = delete;
    MyTcpServer operator=(const MyTcpServer&) = delete;
    QList<MyTcpSocket*> m_tcpSocketList;
};

#endif // MYTCPSERVER_H
