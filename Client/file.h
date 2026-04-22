#ifndef FILE_H
#define FILE_H

#include "protocol.h"
#include "sharefile.h"

#include <QListWidget>
#include <QWidget>

namespace Ui {
class File;
}

class File : public QWidget
{
    Q_OBJECT

public:
    explicit File(QWidget *parent = nullptr);
    QString m_strUserPath;  //用户路径 ./sysfile/用户名
    QString m_strCurPath;   //当前路径
    ShareFile* m_pShareFile;
    QList<FileInfo *> m_pFileInfoList;
    QString m_strUploadPath;
    void mkDir();
    void flushFile();
    void updateFileList(QList<FileInfo*> pFileList);
    void uploadFile();
    ~File();

private slots:
    void on_mkDir_PB_clicked();

    void on_flush_PB_clicked();

    void on_delFile_PB_clicked();

    void on_rename_PB_clicked();

    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_return_PB_clicked();

    void on_upload_PB_clicked();

    void errorBox(QString strMsg);
    void on_share_PB_clicked();

private:
    Ui::File *ui;
};

#endif // FILE_H
