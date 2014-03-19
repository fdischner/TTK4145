// Main application

#include <QCoreApplication>
#include "top.h"

int main(int argc, char *argv[])
{
    pid_t parent_pid = 0;

    QCoreApplication a(argc, argv);

    // If the process is started as a backup, then the parent's pid is passed as an argument
    if (argc > 1)
        parent_pid = (pid_t) strtol(argv[1], NULL, 10);

    // Start the main process
    Top top(argv[0], parent_pid);

    return a.exec();
}
