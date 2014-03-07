#ifndef TOP_H
#define TOP_H

#include <QObject>

class QTimer;
class NetworkManager;
class Control;

class Top : public QObject
{
    Q_OBJECT
public:
    explicit Top(const char *argv0, pid_t parent_pid, QObject *parent = 0);

private:
    NetworkManager *network;
    QTimer *message_timer, *imAlive_timer;
    const char *path;
    pid_t parent_pid;
    Control *control;

signals:

private slots:
    void onMessageReceived(const QByteArray &message);
    void onTakeOver();
    void onSendMessage();
};

#endif // TOP_H
