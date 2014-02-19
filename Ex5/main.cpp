#include <QApplication>
#include "elevator.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	Elevator elev;

	elev.start();
	elev.connect(&elev, SIGNAL(finished()), &a, SLOT(quit()));
	
    return a.exec();
}
