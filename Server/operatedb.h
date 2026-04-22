#ifndef OPERATEDB_H
#define OPERATEDB_H

#include <QObject>
#include <QSqlDatabase>
class OperateDB : public QObject
{
    Q_OBJECT
public:
    ~OperateDB();
    void connect();
    QSqlDatabase m_db;
    static OperateDB& getInstance();
    bool handleRegist(const char* caName, const char* caPwd);
    bool handleLogin(const char* caName, const char* caPwd);
    void handleOffline(const char* caName);
    int handleFindUser(const char* caName);
    QStringList handleOnlineUser();
    int handleAddFriend(char *caCurName, char *caTarName);
    bool handleAddFriendAgree(char *caCurName, char *caTarName);
    QStringList handleFlushFriend(char* caName);
    bool handleDeleteFriend(char* caCurName, char* caDelName);

private:
    explicit OperateDB(QObject *parent = nullptr);
    OperateDB(const OperateDB& instance) = delete;
    OperateDB& operator=(const OperateDB&) = delete;
signals:

};

#endif // OPERATEDB_H
