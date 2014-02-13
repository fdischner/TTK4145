#include "primary.h"
#include <QTimer>
#include <QProcess>

primary::primary(char *argv0, QObject *parent) :
    QObject(parent), path (argv0), cnt_seconds(0), cnt_message(0)
{
    //Open a socket for sending messages to backup process
    network.initSocket(QAbstractSocket::UdpSocket, "localhost", 44445);
    connect(network, SIGNAL(messageReceived()), this, SLOT(onMessageReceived());

    //Create timer for message timeout
    message_timer = new QTimer(this);
    connect(message_timer, SIGNAL(timeout()), this, SLOT(onTakeOver()));
    message_timer->start(150);
}

void primary::onMessageReceived() {
    //Restart timer
    message_timer->start();
}

void primary::onTakeOver() {
    //Become primary process and kill previous primary
    //TODO
    //backup->kill();
    message_timer->stop();

    //Create a backup process
    QProcess *backup = new QProcess();
    QString backup_program = path;
    backup->startDetached(backup_program);

    //Start a timer to send imAlive messages
    imAlive_timer = new QTimer(this);
    connect(imAlive_timer, SIGNAL(timeout()), this, SLOT(onSendMessage()));
    imAlive_timer->start(100);

}

void primary::onSendMessage() {
    //Increment a counter for printing numbers
    cnt_seconds++;

    //Send imAlive message to backup
    network.sendMessage(QByteArray::number(cnt_message));

    //Print number
    if (cnt_seconds == 10) {
        cout << cnt_message;
        cnt_seconds = 0;
    }

}
