#ifndef CHAT_H
#define CHAT_H

#include <QWidget>

namespace Ui {
class Chat;
}

class Chat : public QWidget
{
    Q_OBJECT

public:
    explicit Chat(QWidget *parent = nullptr);
    QString m_strChatName;
    void updateShow_TE(QString strMsg);
    ~Chat();

private slots:
    void on_send_PB_clicked();

private:
    Ui::Chat *ui;
};

#endif // CHAT_H
