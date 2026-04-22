#include "onlineuser.h"
#include "ui_onlineuser.h"
#include "client.h"
OnlineUser::OnlineUser(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OnlineUser)
{
    ui->setupUi(this);
}

OnlineUser::~OnlineUser()
{
    delete ui;
}
void OnlineUser::updateOnlineUser(QStringList nameList)
{
    ui->listWidget->clear();
    ui->listWidget->addItems(nameList);
}

void OnlineUser::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    QString strCurName = Client::getInstance().m_strLoginName;
    QString strTarName = item->text();
    PDU* pdu = mkPDU();
    pdu->uiType = ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;
    memcpy(pdu->caData, strCurName.toStdString().c_str(), 32);
    memcpy(pdu->caData + 32, strTarName.toStdString().c_str(), 32);
    Client::getInstance().sendMsg(pdu);
}
