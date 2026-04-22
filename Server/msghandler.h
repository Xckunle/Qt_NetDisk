#ifndef MSGHANDLER_H
#define MSGHANDLER_H

#include "protocol.h"

#include <QFile>
#include <QString>



class MsgHandler
{
public:
    PDU* pdu;
    MsgHandler();
    PDU* regist();
    PDU* login(QString& strName);
    PDU* findUser();
    PDU* OnlineUser();
    PDU* addFriend();
    PDU* addFriendAgree();
    PDU* flushFriend();
    PDU* deleteFriend();
    PDU* chat();
    PDU* mkdir();
    PDU* flushFile();
    PDU* delFile();
    PDU* renameFile();
    PDU* uploadFileInit();
    PDU* uploadFileData();
    PDU* shareFile();
    PDU* shareFileAgree();
    PDU* uploadFileBreakPoint();
private:
    QFile m_fUploadFile;
    qint64 m_iUploadTotal;
    qint64 m_iUploadRecived;
};

#endif // MSGHANDLER_H
