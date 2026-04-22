#ifndef CLIENT_H
#define CLIENT_H

#include <QTcpSocket>
#include <QWidget>
#include "protocol.h"
#include "reshandler.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Client; }
QT_END_NAMESPACE
//设计模式：
//单例:
//1、懒汉模式：线程安全问题，死锁
//2、饿汉模式：创建类时就创建好了，浪费资源
class Client : public QWidget
{
    //信号槽
    Q_OBJECT

public:
    //Socket套接字
    QString m_strIP;
    quint16 m_usPort;
    QString m_strLoginName;
    QString m_strRootPath;
    ResHandler* m_prh;
    QByteArray buffer;
    QTcpSocket socket;
    PDU* readMsg();
    void handleMsg(PDU* pdu);
    ~Client();
    void loadConfig();
    //单例模式，对外提供一个节点去访问该实例的资源
    static Client& getInstance();

//存放信号槽函数
public slots:
    void showConnect();
    void recvMsg();
    void sendMsg(PDU* pdu);

private slots:
    void on_regist_PB_clicked();

    void on_login_PB_clicked();

private:
    Ui::Client *ui;
    //单例模式，将构造函数私有化，将拷贝构造、赋值运算符删除。
    //QWidget *parent : 对象树，管理栈资源
    Client(QWidget *parent = nullptr);
    Client(const Client&) = delete;
    Client& operator=(Client&) = delete;
};
#endif // CLIENT_H
