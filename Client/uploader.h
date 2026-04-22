#ifndef UPLOADER_H
#define UPLOADER_H

#include "protocol.h"
#include "breakpointmanager.h"

#include <QObject>

class Uploader : public QObject
{
    Q_OBJECT
public:
    explicit Uploader(QObject *parent = nullptr);
    QString m_strUploadPath;       // 本地文件路径
    QString m_strTarPath;          // 服务器目标路径
    QString m_strFileName;         // 文件名
    qint64 m_iTotalSize;           // 文件总大小
    qint64 m_iBreakpoint;          // 断点位置
    bool m_bIsBreakpoint;          // 是否断点续传

    Uploader(QString strFilePath);
    void start();

    // 发送断点续传请求
    void requestBreakpoint();

public slots:
    void uploadFile();
    void handleBreakpointRespond(PDU* pdu);

signals:
    void errorSignal(const QString strMsg);
    void uploadPDU(PDU* pdu);
    void finished();
    void progressSignal(qint64 iUploaded, qint64 iTotal);
    void requestBreakpointSignal(const QString& strFileName, const QString& strTarPath,
                                 qint64 iTotalSize, qint64 iBreakpoint);
};

#endif // UPLOADER_H
