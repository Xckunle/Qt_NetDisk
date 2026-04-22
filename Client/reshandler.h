#ifndef RESHANDLER_H
#define RESHANDLER_H

#include "protocol.h"

#include <QString>



class ResHandler
{
public:
    PDU* pdu;
    ResHandler();
    void regist();
    void login();
    void findUser();
    void OnlineUser();
    void addFriend();
    void addFriendResend();
    void addFriendAgree();
    void flushFriend();
    void deleteFriend();
    void chat();
    void mkdir();
    void flushFile();
    void delFile();
    void renameFile();
    void uploadFileInit();
    void shareFileResend();
    void shareFileAgree();
};

#endif // RESHANDLER_H
