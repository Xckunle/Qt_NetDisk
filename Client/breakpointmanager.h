#ifndef BREAKPOINTMANAGER_H
#define BREAKPOINTMANAGER_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QFile>
#include <QDir>

/**
 * @brief 断点信息结构体
 * 记录每个文件的断点上传信息
 */
struct BreakpointInfo {
    QString strFilePath;      // 文件完整路径
    QString strFileName;      // 文件名
    QString strTarPath;       // 服务器目标路径
    qint64 iTotalSize;        // 文件总大小
    qint64 iUploadedSize;     // 已上传大小
    bool bIsUploading;        // 是否正在上传中
};

class BreakpointManager : public QObject
{
    Q_OBJECT
public:
    static BreakpointManager& getInstance();

    // 检查是否存在未完成的断点续传任务
    bool hasBreakpoint(const QString& strFileName, const QString& strTarPath);

    // 获取断点信息
    BreakpointInfo getBreakpoint(const QString& strFileName, const QString& strTarPath);

    // 保存断点信息
    void saveBreakpoint(const QString& strFileName, const QString& strTarPath,
                       const QString& strLocalPath, qint64 iTotalSize, qint64 iUploadedSize);

    // 更新上传进度
    void updateProgress(const QString& strFileName, const QString& strTarPath, qint64 iUploadedSize);

    // 清除断点记录（上传完成后调用）
    void clearBreakpoint(const QString& strFileName, const QString& strTarPath);

    // 清除所有断点记录
    void clearAllBreakpoints();

    // 获取所有未完成的断点任务
    QList<BreakpointInfo> getAllBreakpoints();

    // 检查文件是否存在
    bool fileExists(const QString& strFilePath);

    // 获取文件大小
    qint64 getFileSize(const QString& strFilePath);

    // 设置正在上传状态
    void setUploading(const QString& strFileName, const QString& strTarPath, bool bUploading);

signals:
    void breakpointReady(const QString& strFileName, const QString& strTarPath, qint64 iUploadedSize);

private:
    explicit BreakpointManager(QObject *parent = nullptr);
    BreakpointManager(const BreakpointManager&) = delete;
    BreakpointManager& operator=(const BreakpointManager&) = delete;

    void loadFromFile();
    void saveToFile();

    QString m_strConfigPath;
    QMap<QString, BreakpointInfo> m_mapBreakpoints;  // key: "文件名|目标路径"
    QString makeKey(const QString& strFileName, const QString& strTarPath);
};

#endif // BREAKPOINTMANAGER_H
