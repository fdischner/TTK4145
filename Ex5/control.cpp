#include "control.h"
#include "elevator.h"

Control::Control(QObject *parent) :
    QObject(parent), wanted_floor(-1)
{
    elevator = new Elevator(this);

    connect(elevator, SIGNAL(floorSensor(int)), this, SLOT(onFloorSensor(int)));
    connect(elevator, SIGNAL(buttonSensor(elev_button_type_t,int)), this, SLOT(onButtonSensor(elev_button_type_t,int)));

    elevator->start();
}

void Control::onFloorSensor(int floor)
{
    if (floor == wanted_floor)
    {
        elevator->setButtonLamp(BUTTON_COMMAND, floor, 0);
        wanted_floor = -1;
    }
}

void Control::onButtonSensor(elev_button_type_t type, int floor)
{
    if (type == BUTTON_COMMAND)
    {
        // TODO: make a queue of floors to visit
        if (wanted_floor != -1)
            elevator->setButtonLamp(BUTTON_COMMAND, wanted_floor, 0);
        wanted_floor = floor;
        elevator->setButtonLamp(BUTTON_COMMAND, wanted_floor, 1);
        elevator->goToFloor(wanted_floor);
    }
}
