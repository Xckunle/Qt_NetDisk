#include "client.h"
#include "ui_client.h"
#include <QFile>
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>
#include "index.h"

Client::Client(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Client)
{
    ui->setupUi(this);
    //加载配置文件
    loadConfig();
    //Socket套接字连接服务器主机
    socket.connectToHost(QHostAddress(m_strIP), m_usPort);
    //发出连接信号
    //&QTcpSocket::connected 发出信号的函数
    //&Client::showConnect 信号槽函数
    connect(&socket, &QTcpSocket::connected, this, &Client::showConnect);
    connect(&socket, &QTcpSocket::readyRead, this, &Client::recvMsg);
    m_prh = new ResHandler;
}

PDU *Client::readMsg()
{
    uint uiPDULen = 0;
    socket.read((char*)&uiPDULen, sizeof(uint));

    uint uiMsgLen = uiPDULen - sizeof(PDU);
    PDU* pdu = mkPDU(uiMsgLen);
    socket.read((char*)pdu + sizeof(uint), uiPDULen - sizeof(uint));
    qDebug() << "recvMsg pdu->uiTotalLen" << pdu->uiTotalLen << endl
             << "pdu->uiMsgLen" << pdu->uiMsgLen << endl
             << "pdu->uiType" << pdu->uiType << endl
             << "pdu->caData" << pdu->caData << endl
             << "pdu->caData" << pdu->caData + 32 << endl
             << "pdu->caMsg" << pdu->caMsg << endl;
    return pdu;
}

//处理消息类型
void Client::handleMsg(PDU *pdu)
{
    qDebug() << "recvMsg pdu->uiTotalLen" << pdu->uiTotalLen << endl
             << "pdu->uiMsgLen" << pdu->uiMsgLen << endl
             << "pdu->uiType" << pdu->uiType << endl
             << "pdu->caData" << pdu->caData << endl
             << "pdu->caData" << pdu->caData + 32 << endl
             << "pdu->caMsg" << pdu->caMsg << endl;
    m_prh->pdu = pdu;
    switch (pdu->uiType) {
    //注册
    case ENUM_MSG_TYPE_REGIST_RESPOND: {
        m_prh->regist();
        break;
    }
    //登录
    case ENUM_MSG_TYPE_LOGIN_RESPOND:{
        m_prh->login();
        break;
    }
    //查找用户
    case ENUM_MSG_TYPE_FIND_USER_RESPOND:{
        m_prh->findUser();
        break;
    }
    //在线用户功能
    case ENUM_MSG_TYPE_ONLINE_USER_RESPOND: {
        m_prh->OnlineUser();
        break;
    }
    //添加好友
    case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND:{
        m_prh->addFriend();
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST: {
        m_prh->addFriendResend();
        break;
    }
    //接受到同意添加好友的请求
    case ENUM_MSG_TYPE_ADD_FRIEND_AGREE_RESPOND: {
        m_prh->addFriendAgree();
        break;
    }
    case ENUM_MSG_TYPE_ADD_FLUSH_FRIEND_RESPOND: {
        m_prh->flushFriend();
        break;
    }
    case ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND: {
        m_prh->deleteFriend();
        break;
    }
    case ENUM_MSG_TYPE_MKDIR_RESPOND: {
        m_prh->mkdir();
        break;
    }
    case ENUM_MSG_TYPE_FLUSH_FILE_RESPOND: {
        m_prh->flushFile();
        break;
    }
    case ENUM_MSG_TYPE_DEL_FILE_RESPOND: {
        m_prh->delFile();
        break;
    }
    case ENUM_MSG_TYPE_RENAME_FILE_RESPOND: {
        m_prh->renameFile();
        break;
    }
    case ENUM_MSG_TYPE_UPLOAD_FILE_INIT_RESPOND: {
        m_prh->uploadFileInit();
        break;
    }
    case ENUM_MSG_TYPE_UPLOAD_FILE_DATA_RESPOND: {
        Index::getInstance().getFile()->flushFile();
        break;
    }
    case ENUM_MSG_TYPE_SHARE_FILE_RESPOND: {
        QMessageBox::information(&Index::getInstance(), "分享文件", "分享已发送");
        break;
    }
    case ENUM_MSG_TYPE_SHARE_FILE_REQUEST: {
        m_prh->shareFileResend();
        break;
    }
    case ENUM_MSG_TYPE_SHARE_FILE_AGREE_RESPOND: {
        m_prh->shareFileAgree();
    }
    default:
        break;
    }
}

Client::~Client()
{
    delete ui;
    delete m_prh;
}

void Client::loadConfig() {
    QFile file(":/connect.config");
    if(file.open(QIODevice::ReadOnly)){
        QString strData = QString(file.readAll());
        //以"\r\n"进行字符串分割，返回值类型是QStringList
        QStringList strList = strData.split("\r\n");
        m_strIP = strList[0];
        m_usPort = strList[1].toUShort();
        m_strRootPath = strList[2];
        qDebug() << "IP:" << m_strIP << " Port:" << m_usPort << "FilePath:" << m_strRootPath;
        file.close();
    }else{
        //消息弹窗
        //模态对话框 和 非模态对话框
        // 阻塞     和 非阻塞
        QMessageBox::warning(this,"提示","打开文件失败");
        qDebug() << "loadConfig open fail";
    }
}

void Client::sendMsg(PDU *pdu)
{
    socket.write((char*)pdu, pdu->uiTotalLen);
    qDebug() << "sendMsg pdu->uiTotalLen" << pdu->uiTotalLen << endl
             << "pdu->uiMsgLen" << pdu->uiMsgLen << endl
             << "pdu->uiType" << pdu->uiType << endl
             << "pdu->caData" << pdu->caData << endl
             << "pdu->caData + 32" << pdu->caData + 32 << endl
             << "pdu->caMsg" << pdu->caMsg << endl;
    delete pdu;
}

Client& Client::getInstance()
{
    static Client instance;
    return instance;
}

void Client::showConnect()
{
    qDebug() << "服务器连接成功";
}

//客户端接收消息函数
void Client::recvMsg()
{
    qDebug() << "recvMsg 接受消息长度" << socket.bytesAvailable();
    QByteArray data = socket.readAll();
    buffer.append(data);
    //粘包、半包问题：
    //1. socket 可能存在多个PDU
    //2. 循环读取 data 判断是不是一个完整的PDU
    //3. 读取一个PDU, 处理一个PDU, 移除一个PDU，直到读取完buffer中的比特流
    while(buffer.size() >= int(sizeof(PDU))) {
        PDU* pdu = (PDU*)buffer.data();
        if(buffer.size() < int(pdu->uiTotalLen)) {
            break;
        }
        handleMsg(pdu);
        buffer.remove(0, pdu->uiTotalLen);
    }
}

//注册功能槽函数
void Client::on_regist_PB_clicked()
{
    QString strName = ui->name_LE->text();
    QString strPwd = ui->pwd_LE->text();
    if(strName.isEmpty() || strPwd.isEmpty()
            || strName.toStdString().size() > 32 || strPwd.toStdString().size() > 32) {
        QMessageBox::information(this, "提示", "用户名或密码错误");
        return;
    }
    PDU* pdu = mkPDU();
    memcpy(pdu->caData, strName.toStdString().c_str(), 32);
    m_strLoginName = strName;
    memcpy(pdu->caData + 32, strPwd.toStdString().c_str(), 32);
    pdu->uiType = ENUM_MSG_TYPE_REGIST_REQUEST;
    sendMsg(pdu);
}

void Client::on_login_PB_clicked()
{
    QString strName = ui->name_LE->text();
    QString strPwd = ui->pwd_LE->text();
    PDU* pdu = mkPDU();
    m_strLoginName = strName;
    memcpy(pdu->caData, strName.toStdString().c_str(), 32);
    memcpy(pdu->caData + 32, strPwd.toStdString().c_str(), 32);
    pdu->uiType = ENUM_MSG_TYPE_LOGIN_REQUEST;
    sendMsg(pdu);
}
