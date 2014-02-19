#include <QApplication>
#include "control.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Control control;

    return a.exec();
}
