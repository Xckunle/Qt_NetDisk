#include "msghandler.h"
#include "mytcpserver.h"
#include "operatedb.h"
#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include "server.h"
MsgHandler::MsgHandler()
{

}

PDU *MsgHandler::regist()
{
    char caName[32] = {'\0'};
    char caPwd[32] = {'\0'};
    memcpy(caName, pdu->caData, 32);
    memcpy(caPwd, pdu->caData + 32, 32);
    bool ret = OperateDB::getInstance().handleRegist(caName, caPwd);
    qDebug() << "regist ret:" << ret;
    if(ret) {
        QDir dir;
        dir.mkdir(QString("%1/%2").arg(Server::getInstance().m_strRootPath).arg(caName));
        qDebug() << "创建用户目录：" << QString("%1/%2").arg(Server::getInstance().m_strRootPath).arg(caName);
    }
    PDU* resPDU = mkPDU();
    resPDU->uiType = ENUM_MSG_TYPE_REGIST_RESPOND;
    memcpy(resPDU->caData, &ret, sizeof(bool) );
    return resPDU;
}

PDU *MsgHandler::login(QString& strName)
{
    char caName[32] = {'\0'};
    char caPwd[32] = {'\0'};
    memcpy(caName, pdu->caData, 32);
    memcpy(caPwd, pdu->caData + 32, 32);
    bool ret = OperateDB::getInstance().handleLogin(caName, caPwd);
    qDebug() << "login ret" << ret;
    if(ret) {
        strName = caName;
    }
    PDU* resPDU = mkPDU();
    resPDU->uiType = ENUM_MSG_TYPE_LOGIN_RESPOND;
    memcpy(resPDU->caData, &ret, sizeof(bool));
    return resPDU;
}

PDU *MsgHandler::findUser()
{
    char caName[32] = {'\0'};
    memcpy(caName, pdu->caData, 32);
    int ret = OperateDB::getInstance().handleFindUser(caName);
    qDebug() << "ret:" << ret;
    PDU* resPDU = mkPDU();
    memcpy(resPDU->caData, &ret, sizeof(int));
    resPDU->uiType = ENUM_MSG_TYPE_FIND_USER_RESPOND;
    return resPDU;
}

PDU *MsgHandler::OnlineUser()
{
    QStringList ret = OperateDB::getInstance().handleOnlineUser();
    qDebug() << "ret.size" << ret.size();
    PDU* resPDU = mkPDU(ret.size() * 32);
    for(int i=0;i<ret.size();i++) {
        qDebug() << "name" << ret[i];
        memcpy(resPDU->caMsg + i * 32,ret[i].toStdString().c_str(), 32);
    }
    resPDU->uiType = ENUM_MSG_TYPE_ONLINE_USER_RESPOND;
    return resPDU;
}

//客户端cur发送加好友请求到服务器，服务器对cur的请求进行转发给客户端tar
PDU *MsgHandler::addFriend()
{
    char caCurName[32] = {'\0'};
    char caTarName[32] = {'\0'};
    memcpy(caCurName, pdu->caData, 32);
    memcpy(caTarName, pdu->caData + 32, 32);
    int ret = OperateDB::getInstance().handleAddFriend(caCurName, caTarName);
    qDebug() << "addFriend ret " << ret;
    //服务器成功处理请求
    if(ret == 1) {
        //转发的pdu->uiType = ENUM_MSG_TYPE_ADD_FRIEND_REQUEST
        MyTcpServer::getInstance().resend(caTarName, pdu);
        return NULL;
    }
    PDU* resPDU = mkPDU();
    memcpy(resPDU->caData, &ret, sizeof(int) );
    resPDU->uiType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
    return resPDU;
}

PDU *MsgHandler::addFriendAgree()
{
    char caCurName[32] = {'\0'};
    char caTarName[32] = {'\0'};
    memcpy(caCurName, pdu->caData, 32);
    memcpy(caTarName, pdu->caData + 32, 32);
    bool ret = OperateDB::getInstance().handleAddFriendAgree(caCurName, caTarName);
    qDebug() << "addFriendAgree ret" << ret;
    PDU* respdu = mkPDU();
    respdu->uiType = ENUM_MSG_TYPE_ADD_FRIEND_AGREE_RESPOND;
    memcpy(respdu->caData, &ret, sizeof(bool));
    MyTcpServer::getInstance().resend(caCurName, respdu);
    return respdu;
}

PDU *MsgHandler::flushFriend()
{
    char caName[32] = {'\0'};
    memcpy(caName, pdu->caData, 32);
    QStringList res = OperateDB::getInstance().handleFlushFriend(caName);
    PDU* respdu = mkPDU(res.size() * 32);
    respdu->uiType = ENUM_MSG_TYPE_ADD_FLUSH_FRIEND_RESPOND;
    for(int i=0;i<res.size();i++) {
        memcpy(respdu->caMsg + i * 32,res[i].toStdString().c_str(), 32);
    }
    return respdu;
}

PDU *MsgHandler::deleteFriend()
{
    char caCurName[32] = {'\0'};
    char caDelName[32] = {'\0'};
    memcpy(caCurName, pdu->caData, 32);
    memcpy(caDelName, pdu->caData + 32, 32);
    bool ret = OperateDB::getInstance().handleDeleteFriend(caCurName, caDelName);
    PDU* respdu = mkPDU();
    respdu->uiType = ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND;
    memcpy(respdu->caData, &ret, sizeof(bool));
    return respdu;
}

PDU *MsgHandler::chat()
{
    char caTarName[32] = {'\0'};
    memcpy(caTarName, pdu->caData + 32, 32);
    MyTcpServer::getInstance().resend(caTarName, pdu);
    return NULL;
}

PDU *MsgHandler::mkdir()
{
    char caDirName[32] = {'\0'};
    memcpy(caDirName, pdu->caData, 32);
    QString strDirPath = QString("%1/%2").arg(pdu->caMsg).arg(caDirName);
    QDir dir;
    bool ret = dir.mkdir(strDirPath);
    qDebug() << "mkdir ret：" << ret;
    PDU* respdu = mkPDU();
    respdu->uiType = ENUM_MSG_TYPE_MKDIR_RESPOND;
    memcpy(respdu->caData, &ret, sizeof(bool));
    return respdu;
}

PDU *MsgHandler::flushFile()
{
    QDir dir(pdu->caMsg);
    //entryInfoList(过滤器：过滤条件)
    //QDir::AllEntries : 列出所有条目
    //QDir::NoDotAndDotDot : 去除当前目录和上级目录
    QFileInfoList fileInfoList = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
    PDU* respdu = mkPDU(sizeof(FileInfo) * fileInfoList.size());
    respdu->uiType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;
    for(int i=0;i<fileInfoList.size();i++) {
        FileInfo* pFileInfo = (FileInfo*)respdu->caMsg + i;
        memcpy(pFileInfo->caName, fileInfoList[i].fileName().toStdString().c_str(), 32);
        //如果是文件夹的话
        if(fileInfoList[i].isDir()) {
            pFileInfo->uiType = 0;
        }else{
            pFileInfo->uiType = 1;
        }
        qDebug() << "caName" << pFileInfo->caName << "uitype" << pFileInfo->uiType;
    }
    return respdu;
}

PDU *MsgHandler::delFile()
{
    char* pPath = pdu->caMsg;
    uint uiType = 0;
    memcpy(&uiType, pdu->caData, sizeof(uint));
    bool ret;
    if(uiType == 0) {
        QDir dir(pPath);
        ret = dir.removeRecursively();
    } else {
        QFile file(pPath);
        ret = file.remove();
    }
    PDU* respdu = mkPDU();
    memcpy(respdu->caData, &ret, sizeof(bool));
    respdu->uiType = ENUM_MSG_TYPE_DEL_FILE_RESPOND;
    return respdu;
}

PDU *MsgHandler::renameFile()
{
    char strOldName[32] = {'\0'};
    char strNewName[32] = {'\0'};
    char* pPath = pdu->caMsg;
    memcpy(strOldName, pdu->caData, 32);
    memcpy(strNewName, pdu->caData + 32, 32);
    QString strOldPath = QString("%1/%2").arg(pPath).arg(strOldName);
    QString strNewPath = QString("%1/%2").arg(pPath).arg(strNewName);
    PDU* respdu = mkPDU(0);
    respdu->uiType = ENUM_MSG_TYPE_RENAME_FILE_RESPOND;
    QDir dir;
    bool ret = dir.rename(strOldPath, strNewPath);
    memcpy(respdu->caData, &ret, sizeof(bool));
    return respdu;
}

PDU *MsgHandler::uploadFileInit()
{
    char caFileName[32] = {'\0'};
    memcpy(caFileName, pdu->caData, 32);
    memcpy(&m_iUploadTotal, pdu->caData + 32, sizeof (qint64));
    m_iUploadRecived = 0;
    QString strPath = QString("%1/%2").arg(pdu->caMsg).arg(caFileName);
    qDebug() << "strPath" << strPath <<endl;
    m_fUploadFile.setFileName(strPath);
    bool ret = m_fUploadFile.open(QIODevice::WriteOnly);
    qDebug() << "ret : " << ret << endl;
    PDU* respdu = mkPDU();
    memcpy(respdu->caData, &ret, sizeof (bool));
    respdu->uiType = ENUM_MSG_TYPE_UPLOAD_FILE_INIT_RESPOND;
    return respdu;
}

PDU *MsgHandler::uploadFileData()
{
    m_fUploadFile.write(pdu->caMsg, pdu->uiMsgLen);
    m_iUploadRecived += pdu->uiMsgLen;
    if(m_iUploadRecived < m_iUploadTotal) {
        return NULL;
    }
    //如果传输结束了
    m_fUploadFile.close();
    m_iUploadRecived = 0;
    PDU* respdu = mkPDU();
    respdu->uiType = ENUM_MSG_TYPE_UPLOAD_FILE_DATA_RESPOND;
    return respdu;
}

PDU *MsgHandler::shareFile()
{
    char strCurName[32] = {'\0'};
    int iFriendSize = 0;
    memcpy(strCurName, pdu->caData, 32);
    memcpy(&iFriendSize, pdu->caData + 32, sizeof (int));
    PDU* resendPDU = mkPDU(pdu->uiMsgLen - iFriendSize * 32);
    resendPDU->uiType = pdu->uiType;
    memcpy(resendPDU->caData, strCurName,32);
    memcpy(resendPDU->caMsg, pdu->caMsg + iFriendSize * 32, pdu->uiMsgLen-iFriendSize * 32);
    char caTmp[32] = {'\0'};
    for(int i=0;i<iFriendSize;i++) {
        memcpy(caTmp, pdu->caMsg + i * 32, 32);
        MyTcpServer::getInstance().resend(caTmp, resendPDU);
    }
    free(resendPDU);
    resendPDU = NULL;
    PDU* respdu = mkPDU();
    respdu->uiType = ENUM_MSG_TYPE_SHARE_FILE_RESPOND;
    return respdu;
}

PDU *MsgHandler::shareFileAgree()
{
    QString strSharedFilePath = pdu->caMsg;
    int index = strSharedFilePath.lastIndexOf('/');
    QString strFileName = strSharedFilePath.right(strSharedFilePath.size() - index - 1);
    QString strTarPath = QString("%1/%2/%3").arg(Server::getInstance().m_strRootPath).arg(pdu->caData).arg(strFileName);
    bool ret = QFile::copy(strSharedFilePath, strTarPath);
    PDU* respdu = mkPDU();
    respdu->uiType = ENUM_MSG_TYPE_SHARE_FILE_AGREE_RESPOND;
    memcpy(respdu->caData, &ret, sizeof (bool));
    return respdu;
}

/**
 * @brief 处理断点续传请求
 * caData: 文件名(32字节) + 目标路径
 * caMsg: 已上传大小(qint64)
 */
PDU *MsgHandler::uploadFileBreakPoint()
{
    char caFileName[32] = {'\0'};
    memcpy(caFileName, pdu->caData, 32);
    QString strTarPath = QString::fromUtf8(pdu->caData + 32);

    qint64 iBreakpoint = 0;
    memcpy(&iBreakpoint, pdu->caMsg, sizeof(qint64));

    QString strFilePath = QString("%1/%2").arg(strTarPath).arg(caFileName);

    qDebug() << "[MsgHandler] 断点续传请求:" << caFileName
             << "目标路径:" << strTarPath << "断点:" << iBreakpoint;

    PDU* respdu = mkPDU();
    respdu->uiType = ENUM_MSG_TYPE_UPLOAD_FILE_BREAKPOINT_RESPOND;

    bool bSuccess = false;
    qint64 iConfirmBreakpoint = 0;

    // 检查文件是否存在
    if (m_fUploadFile.exists()) {
        // 文件存在，检查断点位置是否有效
        qint64 iFileSize = m_fUploadFile.size();

        if (iBreakpoint <= iFileSize) {
            // 断点有效，使用追加模式打开文件
            bSuccess = true;
            iConfirmBreakpoint = iBreakpoint;

            if (!m_fUploadFile.isOpen()) {
                m_fUploadFile.open(QIODevice::Append);
            } else {
                m_fUploadFile.seek(iBreakpoint);
            }

            qDebug() << "[MsgHandler] 断点续传有效，追加写入，文件当前大小:" << iFileSize
                     << "确认断点:" << iConfirmBreakpoint;
        } else {
            // 断点超过文件大小，重新开始
            m_fUploadFile.close();
            m_fUploadFile.setFileName(strFilePath);
            m_fUploadFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
            iConfirmBreakpoint = 0;
            bSuccess = false;

            qDebug() << "[MsgHandler] 断点无效，重新开始上传";
        }
    } else {
        // 文件不存在，需要先初始化
        m_fUploadFile.close();
        m_fUploadFile.setFileName(strFilePath);
        bool bOpened = m_fUploadFile.open(QIODevice::WriteOnly);
        iConfirmBreakpoint = 0;
        bSuccess = bOpened;

        qDebug() << "[MsgHandler] 文件不存在，创建新文件:" << strFilePath;
    }

    // 保存断点信息到成员变量，用于后续uploadFileData
    if (bSuccess) {
        m_iUploadRecived = iConfirmBreakpoint;
    }

    // 打包响应数据：成功标志 + 确认的断点位置
    memcpy(respdu->caData, &bSuccess, sizeof(bool));
    memcpy(respdu->caData + sizeof(bool), &iConfirmBreakpoint, sizeof(qint64));

    return respdu;
}







