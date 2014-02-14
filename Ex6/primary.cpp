#include "primary.h"
#include "networkmanager.h"
#include <QTimer>
#include <QProcess>
#include <iostream>
#include <unistd.h>
#include <signal.h>

primary::primary(char *argv0, pid_t parent_pid, QObject *parent) :
    QObject(parent), path (argv0), parent_pid(parent_pid), cnt_seconds(0), cnt_message(0)
{
    //Open a socket for sending messages to backup process
    network = new NetworkManager(this);
    network->initSocket(QAbstractSocket::UdpSocket, "127.0.0.1", 44445);
    connect(network, SIGNAL(messageReceived(QByteArray)), this, SLOT(onMessageReceived(QByteArray)));

    //Create timer for message timeout
    message_timer = new QTimer(this);
    connect(message_timer, SIGNAL(timeout()), this, SLOT(onTakeOver()));
    message_timer->start(1000);
}

void primary::onMessageReceived(const QByteArray &message) {
    bool ok;
    int i = message.toInt(&ok);

    if (ok)
    {
        //Restart timer
        message_timer->start();
        cnt_message = i;
    }
}

void primary::onTakeOver() {
    //Become primary process and kill previous primary
    if (parent_pid > 0)
        kill(parent_pid, SIGKILL);

    // stop waiting for messages from the master
    message_timer->stop();

    // don't listen for messages anymore
    disconnect(network, SIGNAL(messageReceived(QByteArray)), this, SLOT(onMessageReceived(QByteArray)));

    //Create a backup process
    QProcess *backup = new QProcess();
    QString backup_program = path;
    // pass our pid to backup process
    QStringList args;
    args << QString::number(getpid());
    // start the backup
    backup->startDetached(backup_program, args);

    //Start a timer to send imAlive messages
    imAlive_timer = new QTimer(this);
    connect(imAlive_timer, SIGNAL(timeout()), this, SLOT(onSendMessage()));
    imAlive_timer->start(100);

}

void primary::onSendMessage() {
    //Increment a counter for printing numbers
    cnt_seconds++;

    //Send imAlive message to backup
    network->sendMessage(QByteArray::number(cnt_message));

    //Print number
    if (cnt_seconds == 10) {
        std::cout << cnt_message << std::endl;
        cnt_message++;
        cnt_seconds = 0;
    }

}
