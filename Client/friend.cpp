#include "friend.h"
#include "protocol.h"
#include "ui_friend.h"
#include "client.h"
#include <QInputDialog>
#include <QDebug>
Friend::Friend(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Friend)
{
    ui->setupUi(this);
    m_pOnlineUser = new OnlineUser;
    m_pChat = new Chat;
    flushFriend();
}

Friend::~Friend()
{
    delete ui;
    delete m_pOnlineUser;
    delete m_pChat;
}

void Friend::flushFriend_LW(QStringList friendList)
{
    ui->listWidget->clear();
    ui->listWidget->addItems(friendList);
}

void Friend::flushFriend()
{
    PDU* pdu = mkPDU();
    pdu->uiType = ENUM_MSG_TYPE_ADD_FLUSH_FRIEND_REQUEST;
    memcpy(pdu->caData, Client::getInstance().m_strLoginName.toStdString().c_str(),32);
    Client::getInstance().sendMsg(pdu);
}

QListWidget *Friend::getFriend_LW()
{
    return ui->listWidget;
}

void Friend::on_findUser_PB_clicked()
{
    QString strName = QInputDialog::getText(this, "查找用户", "用户名");
    qDebug() << "strName: " << strName;
    if(strName.isEmpty()) {
        return;
    }
    PDU* pdu = mkPDU();
    memcpy(pdu->caData, strName.toStdString().c_str(), 32);
    pdu->uiType = ENUM_MSG_TYPE_FIND_USER_REQUEST;
    Client::getInstance().sendMsg(pdu);
}

void Friend::on_onlineUser_PB_clicked()
{
    if(m_pOnlineUser->isHidden()) {
        m_pOnlineUser->show();
    }
    PDU* pdu = mkPDU();
    pdu->uiType = ENUM_MSG_TYPE_ONLINE_USER_REQUEST;
    Client::getInstance().sendMsg(pdu);
}

void Friend::on_flush_PB_clicked()
{
    flushFriend();
}

void Friend::on_del_PB_clicked()
{
    QListWidgetItem* pItem = ui->listWidget->currentItem();
    if(!pItem) {
        return;
    }
    QString strTarName = pItem->text();
    PDU* pdu = mkPDU();
    memcpy(pdu->caData, Client::getInstance().m_strLoginName.toStdString().c_str(), 32);
    memcpy(pdu->caData + 32, strTarName.toStdString().c_str(), 32);
    pdu->uiType = ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST;
    Client::getInstance().sendMsg(pdu);
}

void Friend::on_chat_PB_clicked()
{
    QListWidgetItem* pItem = ui->listWidget->currentItem();
    if(!pItem) {
        return;
    }
    m_pChat->m_strChatName = pItem->text();
    if(m_pChat->isHidden()) {
        m_pChat->show();
    }
}
