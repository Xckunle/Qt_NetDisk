#include "file.h"
#include "ui_file.h"
#include "client.h"
#include "protocol.h"
#include "uploader.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
File::File(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::File)
{
    ui->setupUi(this);
    //用户路径
    m_strUserPath = QString("%1/%2").arg(Client::getInstance().m_strRootPath).arg(Client::getInstance().m_strLoginName);
    //当前客户端的文件路径
    m_strCurPath = m_strUserPath;
    //文件和好友的构造函数存在同时给服务器发送socket，这里文件和好友同时执行刷新函数的时候，会有粘包/半包的问题
    flushFile();
    m_pShareFile = new ShareFile;
}

File::~File()
{
    delete ui;
    delete m_pShareFile;
}

void File::mkDir()
{
    QString strDirName = QInputDialog::getText(this, "新建文件夹", "文件夹名：");
    if(strDirName.isEmpty() || strDirName.toStdString().size() > 32 ) {
        QMessageBox::information(this, "提示", "文件名长度非法");
        return;
    }
    PDU* pdu = mkPDU(m_strCurPath.toStdString().size() + 1);
    pdu->uiType = ENUM_MSG_TYPE_MKDIR_REQUEST;
    memcpy(pdu->caData, strDirName.toStdString().c_str(), 32);
    memcpy(pdu->caMsg, m_strCurPath.toStdString().c_str(), m_strCurPath.toStdString().size());
    Client::getInstance().sendMsg(pdu);
}

//刷新文件
void File::flushFile()
{
    PDU* pdu = mkPDU(m_strCurPath.toStdString().size() + 1);
    pdu->uiType = ENUM_MSG_TYPE_FLUSH_FILE_REQUEST;
    memcpy(pdu->caMsg, m_strCurPath.toStdString().c_str(), m_strCurPath.toStdString().size());
    Client::getInstance().sendMsg(pdu);
}

//更新文件目录
void File::updateFileList(QList<FileInfo *> pFileList)
{
    foreach(FileInfo *pFileInfo, m_pFileInfoList) {
        delete pFileInfo;
    }
    m_pFileInfoList = pFileList;
    ui->listWidget->clear();
    //qt的循环遍历写法
    foreach(FileInfo *pFileInfo, pFileList) {
        QListWidgetItem* pItem = new QListWidgetItem;
        if(pFileInfo->uiType == 0) {
            pItem->setIcon(QIcon(QPixmap(":/dir.png")));
        }else if(pFileInfo->uiType == 1){
            pItem->setIcon(QIcon(QPixmap(":/file.png")));
        }
        pItem->setText(pFileInfo->caName);
        qDebug() << "pFileInfo->caName" << pFileInfo->caName;
        ui->listWidget->addItem(pItem);
    }
}

/*
 * 多线程文件上传：
 * 1.socket 不能跨线程，流式传输，保证顺序
 * 2.控件 不能跨线程，要保证主函数循环
*/
void File::uploadFile()
{
    qDebug() << m_strUploadPath << endl;
    Uploader* uploader = new Uploader(m_strUploadPath);
    connect(uploader, &Uploader::errorSignal, this, &File::errorBox, Qt::QueuedConnection);
    connect(uploader, &Uploader::uploadPDU, &Client::getInstance(), &Client::sendMsg, Qt::QueuedConnection);
    connect(uploader,&Uploader::finished,uploader,&Uploader::deleteLater, Qt::QueuedConnection);
    uploader->start();
}

//创建文件夹
void File::on_mkDir_PB_clicked()
{
    mkDir();
    flushFile();
}

//刷新文件
void File::on_flush_PB_clicked()
{
    flushFile();
}

//删除文件
void File::on_delFile_PB_clicked()
{
    QListWidgetItem* pItem = ui->listWidget->currentItem();
    if(!pItem) return;
    int ret = QMessageBox::question(this,"删除文件", QString("是否删除文件 %1").arg(pItem->text()));
    if(ret != QMessageBox::Yes) return;
    QString strPath = QString("%1/%2").arg(m_strCurPath).arg(pItem->text());
    PDU* pdu = mkPDU(strPath.size() + 1);
    pdu->uiType = ENUM_MSG_TYPE_DEL_FILE_REQUEST;
    memcpy(pdu->caMsg, strPath.toStdString().c_str(), strPath.toStdString().size() + 1);
    foreach(FileInfo* pFileInfo, m_pFileInfoList) {
        if(pItem->text() == pFileInfo->caName) {
            memcpy(pdu->caData, &pFileInfo->uiType, sizeof(uint));
            qDebug() << "pFileInfo->uiType" << pFileInfo->uiType;
        }
    }
    Client::getInstance().sendMsg(pdu);
}

//文件重命名
void File::on_rename_PB_clicked()
{
    QListWidgetItem *pItem = ui->listWidget->currentItem();
    if(pItem == NULL) {
        QMessageBox::warning(this, "重命名文件", "请选择要重命名的文件");
    }
    QString strOldName = pItem->text();
    QString strNewName = QInputDialog::getText(this, "重命名文件", "新文件名");
    if(strNewName.isEmpty() || strNewName.size() > 32) {
        QMessageBox::warning(this, "重命名文件", "文件夹名长度非法");
        return;
    }
    PDU* pdu = mkPDU(m_strCurPath.size() + 1);
    pdu->uiType = ENUM_MSG_TYPE_RENAME_FILE_REQUEST;
    memcpy(pdu->caData, strOldName.toStdString().c_str(), strOldName.size());
    memcpy(pdu->caData + 32, strNewName.toStdString().c_str(), strNewName.size());
    memcpy(pdu->caMsg, m_strCurPath.toStdString().c_str(), m_strCurPath.size() );
    Client::getInstance().sendMsg(pdu);
}

void File::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    foreach(FileInfo* pFileInfo, m_pFileInfoList) {
        qDebug() << "pFileInfo->caName" << pFileInfo->caName << endl
                 << "pFileInfo->uiType" << pFileInfo->uiType << endl;
        if(item->text() == pFileInfo->caName && pFileInfo->uiType != 0) {
            return;
        }
    }
    m_strCurPath = QString("%1/%2").arg(m_strCurPath).arg(item->text());
    flushFile();
}

void File::on_return_PB_clicked()
{
    if(m_strCurPath == m_strUserPath) {
        return;
    }
    int index = m_strCurPath.lastIndexOf('/');
    m_strCurPath.remove(index, m_strCurPath.size() - index);
    flushFile();
}

void File::on_upload_PB_clicked()
{
    m_strUploadPath = QFileDialog::getOpenFileName();
    qDebug() << m_strUploadPath;
    int index = m_strUploadPath.lastIndexOf('/');
    QString strFileName = m_strUploadPath.right(m_strUploadPath.size() - index - 1);
    QFile file(m_strUploadPath);
    qint64 iFileSize = file.size();
    qDebug() << iFileSize;
    PDU* pdu = mkPDU(m_strCurPath.toStdString().size() + 1);
    pdu->uiType = ENUM_MSG_TYPE_UPLOAD_FILE_INIT_REQUEST;
    memcpy(pdu->caData, strFileName.toStdString().c_str(), 32);
    memcpy(pdu->caData + 32, &iFileSize, sizeof (qint64));
    memcpy(pdu->caMsg, m_strCurPath.toStdString().c_str(), m_strCurPath.toStdString().size());
    Client::getInstance().sendMsg(pdu);
}

void File::errorBox(QString strMsg)
{
    QMessageBox::information(this, "提示", strMsg);
}

void File::on_share_PB_clicked()
{
    QListWidgetItem* pItem = ui->listWidget->currentItem();
    if(!pItem) {
        return;
    }
    m_pShareFile->m_strFileName = pItem->text();
    m_pShareFile->updateLW();
    if(m_pShareFile->isHidden()) {
        m_pShareFile->show();
    }
}
