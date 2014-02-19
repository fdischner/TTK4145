#include "control.h"
#include "elevator.h"

Control::Control(QObject *parent) :
    QObject(parent)
{
    elevator = new Elevator(this);

    elevator->start();
}
