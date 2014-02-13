#include <QCoreApplication>
#include "primary.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    primary process(argv[0]);

    return a.exec();
}
