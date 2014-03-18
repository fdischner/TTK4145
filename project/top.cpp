#include "top.h"
#include "control.h"
#include "networkmanager.h"
#include <QTimer>
#include <QProcess>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>


Top::Top(const char *argv0, pid_t parent_pid, QObject *parent) :
    QObject(parent), local_network(0), message_timer(0), path (argv0), parent_pid(parent_pid)
{
    if (parent_pid == 0)
    {
        // if we have no parent, then start immediately
        onTakeOver();
    }
    else
    {
        //Open a socket to listen for messages from the parent
        local_network = new NetworkManager(this);
        local_network->initSocket(QAbstractSocket::UdpSocket, "127.0.0.1", 44445);
        connect(local_network, SIGNAL(messageReceived(QByteArray,QHostAddress)), this, SLOT(onMessageReceived(QByteArray,QHostAddress)));

        //Create timer for message timeout
        message_timer = new QTimer(this);
        connect(message_timer, SIGNAL(timeout()), this, SLOT(onTakeOver()));
        // timeout after 1 second
        message_timer->start(1000);
    }
}

void Top::onMessageReceived(const QByteArray &message, const QHostAddress &sender) {
    elev_state = message;

    //Restart timer
    message_timer->start();
}

void Top::onTakeOver() {
    // don't listen for messages anymore
    if (local_network != 0)
        delete local_network;
    local_network = 0;

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

    // start the control module
    control = new Control(elev_state, this);
}
