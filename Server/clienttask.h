#ifndef CLIENTTASK_H
#define CLIENTTASK_H

#include "mytcpsocket.h"

#include <QObject>
#include <QRunnable>

class ClientTask : public QRunnable, public QObject
{
public:
    ClientTask();
    ClientTask(MyTcpSocket* socket);
    MyTcpSocket* m_socket;

    void run() override;
signals:
};

#endif // CLIENTTASK_H
