#include "operatedb.h"
#include <QDebug>
#include <QSqlQuery>
OperateDB::OperateDB(QObject *parent) : QObject(parent)
{
    m_db = QSqlDatabase::addDatabase("QMYSQL");
}

OperateDB::~OperateDB()
{
    m_db.close();
}
//数据库连接
void OperateDB::connect()
{
    m_db.setHostName("localhost");
    m_db.setPort(3306);
    m_db.setUserName("root");
    m_db.setPassword("123456");
    m_db.setDatabaseName("qt_project");
    if(m_db.open()) {
        qDebug() << "数据库连接成功";
    }else {
        qDebug() << "数据库连接失败";
    }
}

OperateDB &OperateDB::getInstance()
{
    static OperateDB instance;
    return instance;
}
//处理注册功能函数
bool OperateDB::handleRegist(const char *caName, const char *caPwd)
{
    if(caName == NULL || caPwd == NULL) {
        return false;
    }
    QString sql = QString("select * from user_info where name='%1'").arg(caName);
    qDebug() << "查询用户是否存在 sql" << sql << endl;
    QSqlQuery q;
    if(!q.exec(sql)) {
        return false;
    }
    if(q.next()) {
        return false;
    }
    sql = QString("insert into user_info(name, pwd) values('%1','%2')").arg(caName).arg(caPwd);
    qDebug() << "插入一条新用户 sql" << sql << endl;
    return q.exec(sql);
}
//处理登录功能函数
bool OperateDB::handleLogin(const char *caName, const char *caPwd)
{
    if(caName == NULL || caPwd == NULL) {
        return false;
    }
    QString sql = QString("select * from user_info where name = '%1' and pwd = '%2'").arg(caName).arg(caPwd);
    qDebug() << "查询该用户是否存在 sql" << sql << endl;
    QSqlQuery q;
    if(!q.exec(sql)) {
        return false;
    }
    if(!q.next()) {
        return false;
    }
    sql = QString("update user_info set online = 1 where name = '%1' and pwd = '%2'").arg(caName).arg(caPwd);
    qDebug() << "更新用户在线状态为1 sql" << sql << endl;
    return q.exec(sql);
}

void OperateDB::handleOffline(const char *caName)
{
    if(caName == NULL) return;
    QString sql = QString("update user_info set online = 0 where name = '%1'").arg(caName);
    QSqlQuery q;
    bool ret = q.exec(sql);
    if(ret) {
        qDebug() << "更新用户在线状态为0 sql" << sql << endl;
    }
}
/*
 * 查找好友功能
 * 状态值：
 * 0：用户不在线
 * 1：用户在线
 * 2：用户不存在
*/
int OperateDB::handleFindUser(const char *caName)
{
    if(caName == NULL) return -1;
    QString sql = QString("select online from user_info where name = '%1'").arg(caName);
    QSqlQuery q;
    q.exec(sql);
    if(q.next()) {
        return q.value(0).toInt();
    }
    return 2;
}

QStringList OperateDB::handleOnlineUser()
{
    QStringList result;
    result.clear();
    QString sql = QString("select name from user_info where online = 1");
    QSqlQuery q;
    q.exec(sql);
    qDebug() << "查询在线用户sql " << sql;
    while(q.next()) {
        result.append(q.value(0).toString());
    }
    return result;
}

int OperateDB::handleAddFriend(char *caCurName, char *caTarName)
{
    if(caCurName == NULL || caTarName == NULL) {
        return -3;
    }
    QString sql = QString(R"(
                          select * from friend
                          where
                          (
                          user_id=(select id from user_info where name='%1')
                          and
                          friend_id=(select id from user_info where name='%2')
                          )
                          or
                          (
                          user_id=(select id from user_info where name='%2')
                          and
                          friend_id=(select id from user_info where name='%1')
    );)").arg(caCurName).arg(caTarName);
    qDebug() << "查询是否是好友 sql" << endl;
    QSqlQuery q;
    q.exec(sql);
    //该用户存在，证明已经是好友
    if(q.next()) {
        return -2;
    }
    //查看用户是否在线
    sql = QString("select online from user_info where name = '%1'").arg(caTarName);
    q.exec(sql);
    if(q.next()) {
        return q.value(0).toInt();
    }
    return -1;
}

bool OperateDB::handleAddFriendAgree(char *caCurName, char *caTarName)
{
    if(caCurName == NULL || caTarName == NULL) {
        return false;
    }
    QString sql = QString(R"(
            insert into friend(user_id, friend_id)
            select u1.id, u2.id from user_info u1, user_info u2
            where
            u1.name = '%1' and u2.name = '%2';
            )").arg(caCurName).arg(caTarName);
    qDebug() << "cur :" << caCurName << " " << "tar" << caTarName;
    qDebug() << "处理添加好友申请 sql" << endl;
    QSqlQuery q;
    return q.exec(sql);
}

QStringList OperateDB::handleFlushFriend(char *caName)
{
    QStringList res;
    if(caName == NULL) {
        return res;
    }
    QString sql = QString(R"(select name from user_info where id in (
          select user_id from friend where friend_id=(select id from user_info where name='%1')
                          union
          select friend_id from friend where user_id=(select id from user_info where name='%1')
        )
    )").arg(caName);
    qDebug() << "刷新好友列表申请 sql" << endl;
    QSqlQuery q;
    q.exec(sql);
    while(q.next()) {
        res.append(q.value(0).toString());
    }
    return res;
}

bool OperateDB::handleDeleteFriend(char *caCurName, char *caDelName)
{
    if(caCurName == NULL || caDelName == NULL) {
        return false;
    }
    QString sql = QString(R"(
         delete from friend
             where
              (
                 user_id=(select id from user_info where name='%1')
                 and
                 friend_id=(select id from user_info where name='%2')
              )
              or
              (
                 user_id=(select id from user_info where name='%2')
                 and
                 friend_id=(select id from user_info where name='%1')
              )
    )").arg(caCurName).arg(caDelName);
    QSqlQuery q;
    if(q.exec(sql)) {
        return q.numRowsAffected() > 0;
    }
    return false;
}




