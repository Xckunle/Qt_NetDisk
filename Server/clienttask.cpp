#include "clienttask.h"

ClientTask::ClientTask()
{

}

ClientTask::ClientTask(MyTcpSocket *socket)
{
    m_socket = socket;
}

void ClientTask::run()
{
    connect(m_socket, &QTcpSocket::readyRead, m_socket, &MyTcpSocket::recvMsg, Qt::QueuedConnection);
    connect(m_socket, &QTcpSocket::disconnected, m_socket, &MyTcpSocket::clientOffline,Qt::QueuedConnection);
}
