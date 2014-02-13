#ifndef PRIMARY_H
#define PRIMARY_H

#include <QObject>

class QTimer;
class NetworkManager;

class primary : public QObject
{
    Q_OBJECT

public:
    explicit primary(char *argv0, QObject *parent = 0);

private:
    NetworkManager *network;
    QTimer *message_timer, *imAlive_timer;
    char *path;
    int cnt_seconds, cnt_message;

signals:

private slots:
    void onMessageReceived(const QByteArray &message);
    void onTakeOver();
    void onSendMessage();
};

#endif // PRIMARY_H
