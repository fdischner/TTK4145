// Top class includes the behavior of both the main and backup processes

#ifndef TOP_H
#define TOP_H

#include <QObject>
#include <QHostAddress>

class QTimer;
class NetworkManager;
class Control;

class Top : public QObject
{
    Q_OBJECT

public:
    explicit Top(const char *argv0, pid_t parent_pid, QObject *parent = 0);

signals:

private slots:
    void onMessageReceived(const QByteArray &message, const QHostAddress &sender);
    void onTakeOver();

private:
    NetworkManager *local_network;
    Control *control;
    QTimer *message_timer;
    QByteArray elev_state;
    pid_t parent_pid;
    const char *path;
};

#endif
