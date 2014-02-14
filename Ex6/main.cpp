#include <QCoreApplication>
#include "primary.h"
#include <inttypes.h>

int main(int argc, char *argv[])
{
    pid_t parent_pid = 0;

    QCoreApplication a(argc, argv);

    // if we are started as a backup, then the parent's pid is passed as an argument
    if (argc > 1)
        parent_pid = (pid_t) strtol(argv[1], NULL, 10);

    primary process(argv[0], parent_pid);

    return a.exec();
}
