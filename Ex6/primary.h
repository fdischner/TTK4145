#ifndef PRIMARY_H
#define PRIMARY_H

#include <QObject>
#include "networkmanager.h"

class QTimer;

class primary : public QObject
{
    Q_OBJECT

public:
    explicit primary(char *argv0, QObject *parent = 0);

private:
    NetworkManager network;
    QTimer *message_timer, *imAlive_timer;
    char *path;
    int cnt_seconds, cnt_message;

signals:

public slots:
    void onMessageReceived();
    void onTakeOver();
    void onSendMessage();
};

#endif // PRIMARY_H
