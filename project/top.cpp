#include "top.h"
#include "control.h"
#include "networkmanager.h"
#include <QTimer>
#include <QProcess>
#include <signal.h>

Top::Top(const char *argv0, pid_t parent_pid, QObject *parent) :
    QObject(parent), message_timer(0), path (argv0), parent_pid(parent_pid)
{
    //Open a socket for sending messages to backup process
    network = new NetworkManager(this);
    network->initSocket(QAbstractSocket::UdpSocket, "127.0.0.1", 44445);
    connect(network, SIGNAL(messageReceived(QByteArray)), this, SLOT(onMessageReceived(QByteArray)));

    if (parent_pid == 0)
    {
        // if we have no parent, then start immediately
        onTakeOver();
    }
    else
    {
        //Create timer for message timeout
        message_timer = new QTimer(this);
        connect(message_timer, SIGNAL(timeout()), this, SLOT(onTakeOver()));
        // timeout after 1 second
        message_timer->start(1000);
    }
}

void Top::onMessageReceived(const QByteArray &message) {
    // TODO: should we check the message content?
    //Restart timer
    message_timer->start();
}

void Top::onTakeOver() {
    // don't listen for messages anymore
    disconnect(network, SIGNAL(messageReceived(QByteArray)), this, SLOT(onMessageReceived(QByteArray)));

    // stop waiting for messages from the master
    if (message_timer != 0)
        message_timer->stop();

    //Become primary process and kill previous primary
    if (parent_pid > 0)
        kill(parent_pid, SIGKILL);

    //Create a backup process
    QProcess *backup = new QProcess();
    QString backup_program(path);
    // pass our pid to backup process
    QStringList args;
    args << QString::number(getpid());
    // start the backup
    backup->startDetached(backup_program, args);

    //Start a timer to send imAlive messages
    imAlive_timer = new QTimer(this);
    connect(imAlive_timer, SIGNAL(timeout()), this, SLOT(onSendMessage()));
    // send alive messages every 100ms
    imAlive_timer->start(100);

    // start the control module
    control = new Control(this);
}

void Top::onSendMessage() {
    // Send imAlive message to backup
    // TODO: we should probably send the current state here
    network->sendMessage(QByteArray("alive"));
}
