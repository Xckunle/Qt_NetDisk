#include "mytcpsocket.h"

#include "operatedb.h"
#include "mytcpserver.h"
#include <QtDebug>
MyTcpSocket::MyTcpSocket()
{
    connect(this, &QTcpSocket::readyRead, this, &MyTcpSocket::recvMsg);
    connect(this, &QTcpSocket::disconnected, this, &MyTcpSocket::clientOffline);
    m_pmh = new MsgHandler;
}

MyTcpSocket::~MyTcpSocket()
{
    delete m_pmh;
}

void MyTcpSocket::recvMsg()
{
    qDebug() << "recvMsg 接受消息长度" << this->bytesAvailable();
    QByteArray data = this->readAll();
    buffer.append(data);
    while(buffer.size() >= int(sizeof(PDU))) {
        PDU* pdu = (PDU*)buffer.data();
        if(buffer.size() < int(pdu->uiTotalLen)) {
            break;
        }
        PDU* respdu = handleMsg(pdu);
        if(respdu) {
            sendMsg(respdu);
        }
        buffer.remove(0, pdu->uiTotalLen);
    }
}

void MyTcpSocket::sendMsg(PDU* pdu)
{
    this->write((char*)pdu, pdu->uiTotalLen);
    qDebug() << "send msg resPDU->uiTotalLen" << pdu->uiTotalLen << endl
             << "resPDU->uiMsgLen" << pdu->uiMsgLen << endl
             << "resPDU->uiType" << pdu->uiType << endl
             << "resPDU->caData" << pdu->caData << endl
             << "resPDU->caData + 32" << pdu->caData + 32 << endl
             << "resPDU->caMsg" << pdu->caMsg << endl;
}

PDU *MyTcpSocket::readMsg()
{
    qDebug() << "recvMsg接收的长度 " << this->bytesAvailable();
    //数据流，根据定义的数据协议单元，数据总长度的信息存储在sizeof(uint)字节长度里
    uint uiPDULen = 0;
    this->read((char*)&uiPDULen,sizeof(uint));
    uint uiMsgLen = uiPDULen - sizeof(PDU);
    PDU* pdu = mkPDU(uiMsgLen);
    this->read((char*)pdu + sizeof(uint), uiPDULen-sizeof(uint));
    return pdu;
}

PDU *MyTcpSocket::handleMsg(PDU *pdu)
{
    qDebug() << "recvMsg pdu->uiTotalLen" << pdu->uiTotalLen << endl
             << "pdu->uiMsgLen" << pdu->uiMsgLen << endl
             << "pdu->uiType" << pdu->uiType << endl
             << "pdu->caData" << pdu->caData << endl
             << "pdu->caData + 32" << pdu->caData + 32 << endl
             << "pdu->caMsg" << pdu->caMsg << endl;
    PDU* resPDU = NULL;
    m_pmh->pdu = pdu;
    //判断信息类型，作出不同的操作
    //服务端响应并返回对应的信息
    switch (pdu->uiType) {
    case ENUM_MSG_TYPE_REGIST_REQUEST: {
        resPDU = m_pmh->regist();
        break;
    }
    case ENUM_MSG_TYPE_LOGIN_REQUEST: {
        resPDU = m_pmh->login(m_strLoginName);
        break;
    }
    case ENUM_MSG_TYPE_FIND_USER_REQUEST:{
        resPDU = m_pmh->findUser();
        break;
    }
    case ENUM_MSG_TYPE_ONLINE_USER_REQUEST: {
        resPDU = m_pmh->OnlineUser();
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST: {
        //resPDU->uiType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND
        resPDU = m_pmh->addFriend();
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_AGREE_REQUEST: {
        resPDU = m_pmh->addFriendAgree();
        break;
    }
    case ENUM_MSG_TYPE_ADD_FLUSH_FRIEND_REQUEST: {
        resPDU = m_pmh->flushFriend();
        break;
    }
    case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST: {
        resPDU = m_pmh->deleteFriend();
        break;
    }
    case ENUM_MSG_TYPE_CHAT_REQUEST: {
        resPDU = m_pmh->chat();
        break;
    }
    case ENUM_MSG_TYPE_MKDIR_REQUEST: {
        resPDU = m_pmh->mkdir();
        break;
    }
    case ENUM_MSG_TYPE_FLUSH_FILE_REQUEST: {
        resPDU = m_pmh->flushFile();
        break;
    }
    case ENUM_MSG_TYPE_DEL_FILE_REQUEST: {
        resPDU = m_pmh->delFile();
        break;
    }
    case ENUM_MSG_TYPE_RENAME_FILE_REQUEST: {
        resPDU = m_pmh->renameFile();
        break;
    }
    case ENUM_MSG_TYPE_UPLOAD_FILE_INIT_REQUEST: {
        resPDU = m_pmh->uploadFileInit();
        break;
    }
    case ENUM_MSG_TYPE_UPLOAD_FILE_DATA_REQUEST: {
        resPDU = m_pmh->uploadFileData();
        break;
    }
    case ENUM_MSG_TYPE_SHARE_FILE_REQUEST: {
        resPDU = m_pmh->shareFile();
        break;
    }
    case ENUM_MSG_TYPE_SHARE_FILE_AGREE_REQUEST: {
        resPDU = m_pmh->shareFileAgree();
        break;
    }
    case ENUM_MSG_TYPE_UPLOAD_FILE_BREAKPOINT_REQUEST: {
        resPDU = m_pmh->uploadFileBreakPoint();
    }
    default:
        break;
    }
    return resPDU;
}

void MyTcpSocket::clientOffline()
{
    OperateDB::getInstance().handleOffline(m_strLoginName.toStdString().c_str());
    MyTcpServer::getInstance().RemoveSocket(this);
}
