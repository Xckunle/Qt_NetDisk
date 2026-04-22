#include "breakpointmanager.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QCoreApplication>

BreakpointManager::BreakpointManager(QObject *parent)
    : QObject(parent)
{
    // 配置文件路径：应用程序目录/breakpoint.dat
    m_strConfigPath = QCoreApplication::applicationDirPath() + "/breakpoint.dat";
    loadFromFile();
}

/**
 * @brief 获取单例实例
 */
BreakpointManager& BreakpointManager::getInstance()
{
    static BreakpointManager instance;
    return instance;
}

/**
 * @brief 生成唯一键值
 */
QString BreakpointManager::makeKey(const QString& strFileName, const QString& strTarPath)
{
    return strFileName + "|" + strTarPath;
}

/**
 * @brief 检查是否存在断点记录
 */
bool BreakpointManager::hasBreakpoint(const QString& strFileName, const QString& strTarPath)
{
    QString key = makeKey(strFileName, strTarPath);
    if (m_mapBreakpoints.contains(key)) {
        BreakpointInfo info = m_mapBreakpoints[key];
        // 检查文件是否还存在且未上传完成
        if (info.bIsUploading && fileExists(info.strFilePath)) {
            return true;
        }
    }
    return false;
}

/**
 * @brief 获取断点信息
 */
BreakpointInfo BreakpointManager::getBreakpoint(const QString& strFileName, const QString& strTarPath)
{
    QString key = makeKey(strFileName, strTarPath);
    if (m_mapBreakpoints.contains(key)) {
        return m_mapBreakpoints[key];
    }
    return BreakpointInfo();
}

/**
 * @brief 保存断点信息
 */
void BreakpointManager::saveBreakpoint(const QString& strFileName, const QString& strTarPath,
                                      const QString& strLocalPath, qint64 iTotalSize, qint64 iUploadedSize)
{
    QString key = makeKey(strFileName, strTarPath);
    BreakpointInfo info;
    info.strFilePath = strLocalPath;
    info.strFileName = strFileName;
    info.strTarPath = strTarPath;
    info.iTotalSize = iTotalSize;
    info.iUploadedSize = iUploadedSize;
    info.bIsUploading = true;

    m_mapBreakpoints[key] = info;
    saveToFile();

    qDebug() << "[BreakpointManager] 保存断点:" << strFileName
             << "已上传:" << iUploadedSize << "/" << iTotalSize;
}

/**
 * @brief 更新上传进度
 */
void BreakpointManager::updateProgress(const QString& strFileName, const QString& strTarPath, qint64 iUploadedSize)
{
    QString key = makeKey(strFileName, strTarPath);
    if (m_mapBreakpoints.contains(key)) {
        m_mapBreakpoints[key].iUploadedSize = iUploadedSize;
        saveToFile();

        qDebug() << "[BreakpointManager] 更新进度:" << strFileName
                 << "已上传:" << iUploadedSize;
    }
}

/**
 * @brief 清除断点记录
 */
void BreakpointManager::clearBreakpoint(const QString& strFileName, const QString& strTarPath)
{
    QString key = makeKey(strFileName, strTarPath);
    if (m_mapBreakpoints.contains(key)) {
        m_mapBreakpoints[key].bIsUploading = false;
        m_mapBreakpoints.remove(key);
        saveToFile();

        qDebug() << "[BreakpointManager] 清除断点:" << strFileName;
    }
}

/**
 * @brief 清除所有断点记录
 */
void BreakpointManager::clearAllBreakpoints()
{
    m_mapBreakpoints.clear();
    saveToFile();
    qDebug() << "[BreakpointManager] 清除所有断点记录";
}

/**
 * @brief 获取所有未完成的断点任务
 */
QList<BreakpointInfo> BreakpointManager::getAllBreakpoints()
{
    QList<BreakpointInfo> list;
    for (auto it = m_mapBreakpoints.begin(); it != m_mapBreakpoints.end(); ++it) {
        if (it.value().bIsUploading) {
            list.append(it.value());
        }
    }
    return list;
}

/**
 * @brief 检查文件是否存在
 */
bool BreakpointManager::fileExists(const QString& strFilePath)
{
    return QFile::exists(strFilePath);
}

/**
 * @brief 获取文件大小
 */
qint64 BreakpointManager::getFileSize(const QString& strFilePath)
{
    QFile file(strFilePath);
    if (file.open(QIODevice::ReadOnly)) {
        qint64 size = file.size();
        file.close();
        return size;
    }
    return 0;
}

/**
 * @brief 设置上传状态
 */
void BreakpointManager::setUploading(const QString& strFileName, const QString& strTarPath, bool bUploading)
{
    QString key = makeKey(strFileName, strTarPath);
    if (m_mapBreakpoints.contains(key)) {
        m_mapBreakpoints[key].bIsUploading = bUploading;
        saveToFile();
    }
}

/**
 * @brief 从文件加载断点数据
 */
void BreakpointManager::loadFromFile()
{
    QFile file(m_strConfigPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return;
    }

    QJsonObject root = doc.object();
    QJsonObject breakpoints = root["breakpoints"].toObject();

    for (auto it = breakpoints.begin(); it != breakpoints.end(); ++it) {
        QJsonObject obj = it.value().toObject();
        BreakpointInfo info;
        info.strFilePath = obj["filePath"].toString();
        info.strFileName = obj["fileName"].toString();
        info.strTarPath = obj["tarPath"].toString();
        info.iTotalSize = obj["totalSize"].toVariant().toLongLong();
        info.iUploadedSize = obj["uploadedSize"].toVariant().toLongLong();
        info.bIsUploading = obj["isUploading"].toBool();

        m_mapBreakpoints[it.key()] = info;
    }

    qDebug() << "[BreakpointManager] 加载断点数据完成, 记录数:" << m_mapBreakpoints.size();
}

/**
 * @brief 保存断点数据到文件
 */
void BreakpointManager::saveToFile()
{
    QJsonObject root;
    QJsonObject breakpoints;

    for (auto it = m_mapBreakpoints.begin(); it != m_mapBreakpoints.end(); ++it) {
        QJsonObject obj;
        obj["filePath"] = it.value().strFilePath;
        obj["fileName"] = it.value().strFileName;
        obj["tarPath"] = it.value().strTarPath;
        obj["totalSize"] = QString::number(it.value().iTotalSize);
        obj["uploadedSize"] = QString::number(it.value().iUploadedSize);
        obj["isUploading"] = it.value().bIsUploading;

        breakpoints[it.key()] = obj;
    }

    root["breakpoints"] = breakpoints;

    QFile file(m_strConfigPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "[BreakpointManager] 保存断点数据失败";
        return;
    }

    file.write(QJsonDocument(root).toJson());
    file.close();
}
