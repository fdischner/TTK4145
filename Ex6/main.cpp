#include <QCoreApplication>
#include <QProcess>
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QObject *parent;

    std::cout << "hola";

    //Check if there are any other processes and kill them


    //Create a backup process
    QProcess *backup = new QProcess(parent);
    QString backup_program = "";
    backup->startDetached(backup_program);

    //Do something
    //while(1) {


        //Send imAlive messages to backup process

    //}

    backup->kill();

    return a.exec();
}
