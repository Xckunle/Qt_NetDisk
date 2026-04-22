#ifndef SERVER_H
#define SERVER_H

#include <QWidget>
#include "mytcpserver.h"
class Server : public QWidget
{
    Q_OBJECT

public:
    ~Server();
    QString m_strIP;
    quint16 m_usPort;
    QString m_strRootPath;
    static Server& getInstance();
    void loadConfig();

private:
    Server(QWidget *parent = nullptr);
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
};
#endif // SERVER_H
