#include "server.h"

#include <QFile>
#include <QDebug>
#include <QMessageBox>
Server::Server(QWidget *parent)
    : QWidget(parent)
{
    //加载配置文件
    loadConfig();
    //单例模式 监听 发送socket的客户端
    MyTcpServer::getInstance().listen(QHostAddress(m_strIP), m_usPort);
}

Server& Server::getInstance()
{
    static Server instance;
    return instance;
}

Server::~Server()
{
}

void Server::loadConfig()
{
    QFile file(":/connect.config");
    if(file.open(QIODevice::ReadOnly)){
        QString strData = QString(file.readAll());
        QStringList strList = strData.split("\r\n");
        m_strIP = strList[0];
        m_usPort = strList[1].toUShort();
        m_strRootPath = strList[2];
        qDebug() << "IP :" << m_strIP << " Port :" << m_usPort << "FilePath:" << m_strRootPath;
        file.close();
    }else{
        QMessageBox::critical(this, "提示", "打开文件失败");
        qDebug() << "loadConfig 打开文件失败";
    }
}

