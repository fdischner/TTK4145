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

private:
    NetworkManager *local_network;
    QTimer *message_timer;
    const char *path;
    pid_t parent_pid;
    Control *control;
    QByteArray elev_state;

signals:

private slots:
    void onMessageReceived(const QByteArray &message, const QHostAddress &sender);
    void onTakeOver();
};

#endif // TOP_H
