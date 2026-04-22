#include "protocol.h"
#include "uploader.h"

#include <QFile>
#include <QIODevice>
#include <QThread>
#include <QDebug>

Uploader::Uploader(QObject *parent) : QObject(parent)
    , m_iTotalSize(0)
    , m_iBreakpoint(0)
    , m_bIsBreakpoint(false)
{

}

Uploader::Uploader(QString strFilePath)
{
    // 初始化读取文件的路径
    m_strUploadPath = strFilePath;
    m_strFileName = strFilePath.mid(strFilePath.lastIndexOf('/') + 1);
    m_iTotalSize = BreakpointManager::getInstance().getFileSize(strFilePath);
    m_iBreakpoint = 0;
    m_bIsBreakpoint = false;
}

void Uploader::start()
{
    QThread* thread = new QThread;
    this->moveToThread(thread);
    connect(thread, &QThread::started, this, &Uploader::uploadFile, Qt::QueuedConnection);
    connect(this, &Uploader::finished, thread, &QThread::quit, Qt::QueuedConnection);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater, Qt::QueuedConnection);
    thread->start();
}

/**
 * @brief 请求断点续传
 */
void Uploader::requestBreakpoint()
{
    // 检查是否存在断点记录
    if (BreakpointManager::getInstance().hasBreakpoint(m_strFileName, m_strTarPath)) {
        BreakpointInfo info = BreakpointManager::getInstance().getBreakpoint(m_strFileName, m_strTarPath);
        m_iBreakpoint = info.iUploadedSize;
        m_bIsBreakpoint = true;

        qDebug() << "[Uploader] 发现断点:" << m_strFileName
                 << "已上传:" << m_iBreakpoint;

        emit requestBreakpointSignal(m_strFileName, m_strTarPath, m_iTotalSize, m_iBreakpoint);
    } else {
        // 无断点，从头开始
        m_iBreakpoint = 0;
        m_bIsBreakpoint = false;

        // 保存初始断点信息
        BreakpointManager::getInstance().saveBreakpoint(
            m_strFileName, m_strTarPath, m_strUploadPath, m_iTotalSize, 0);

        emit requestBreakpointSignal(m_strFileName, m_strTarPath, m_iTotalSize, 0);
    }
}

/**
 * @brief 处理断点续传响应
 */
void Uploader::handleBreakpointRespond(PDU* pdu)
{
    bool bSuccess = false;
    memcpy(&bSuccess, pdu->caData, sizeof(bool));

    if (!bSuccess) {
        // 服务器端文件不存在或校验失败，从头开始上传
        m_iBreakpoint = 0;
        m_bIsBreakpoint = false;

        qDebug() << "[Uploader] 断点续传失败，从头开始上传:" << m_strFileName;
    } else {
        // 断点续传成功
        memcpy(&m_iBreakpoint, pdu->caData + sizeof(bool), sizeof(qint64));

        qDebug() << "[Uploader] 断点续传成功，从" << m_iBreakpoint << "字节处继续:" << m_strFileName;
    }

    // 开始上传
    uploadFile();
}

void Uploader::uploadFile()
{
    QFile file(m_strUploadPath);
    if(!file.open(QIODevice::ReadOnly)) {
        emit errorSignal("上传文件失败");
        emit finished();
        return;
    }

    // 如果是断点续传，从断点位置开始读取
    if (m_iBreakpoint > 0) {
        file.seek(m_iBreakpoint);
        qDebug() << "[Uploader] 从断点" << m_iBreakpoint << "开始上传:" << m_strFileName;
    }

    qint64 iUploaded = m_iBreakpoint;
    const qint64 CHUNK_SIZE = 4096;

    while(true) {
        PDU* pdu = mkPDU(CHUNK_SIZE);
        pdu->uiType = ENUM_MSG_TYPE_UPLOAD_FILE_DATA_REQUEST;

        qint64 ret = file.read(pdu->caMsg, CHUNK_SIZE);
        if(ret < 0) {
            emit errorSignal("上传文件失败");
            file.close();
            break;
        }
        if(ret == 0) {
            break;
        }

        pdu->uiMsgLen = ret;
        pdu->uiTotalLen = ret + sizeof(PDU);

        // 记录已上传位置（每次发送后更新）
        iUploaded += ret;

        // 更新断点进度
        BreakpointManager::getInstance().updateProgress(m_strFileName, m_strTarPath, iUploaded);

        // 发送进度信号
        emit progressSignal(iUploaded, m_iTotalSize);

        emit uploadPDU(pdu);
    }

    file.close();

    // 上传完成，清除断点记录
    if (iUploaded >= m_iTotalSize) {
        BreakpointManager::getInstance().clearBreakpoint(m_strFileName, m_strTarPath);
        qDebug() << "[Uploader] 上传完成，清除断点记录:" << m_strFileName;
    }

    emit finished();
}

