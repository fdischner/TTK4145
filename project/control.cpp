#include "control.h"
#include "elevator.h"

Control::Control(QObject *parent) :
    QObject(parent)
{
    for (int i = 0; i < N_FLOORS; i++) {
        call[0][i] = false;
        call[1][i] = false;
        call[2][i] = false;
    }

    elevator = new Elevator(this);

    connect(elevator, SIGNAL(floorSensor(int)), this, SLOT(onFloorSensor(int)));
    connect(elevator, SIGNAL(buttonSensor(elev_button_type_t,int)), this, SLOT(onButtonSensor(elev_button_type_t,int)));

    elevator->start();
}

void Control::onFloorSensor(int floor)
{
    if (call[BUTTON_COMMAND][floor] ||
        (elevator->direction == 1 && call[BUTTON_CALL_UP][floor]) ||
        (elevator->direction == -1 && call[BUTTON_CALL_DOWN][floor]))
    {
        elevator->stop();
        elevator->setButtonLamp(BUTTON_COMMAND, floor, 0);
        call[BUTTON_COMMAND][floor] = false;

        if (elevator->direction == 1)
        {
            elevator->setButtonLamp(BUTTON_CALL_UP, floor, 0);
            call[BUTTON_CALL_UP][floor] = false;
        }

        if (elevator->direction == -1)
        {
            elevator->setButtonLamp(BUTTON_CALL_DOWN, floor, 0);
            call[BUTTON_CALL_DOWN][floor] = false;
        }
    }
}

void Control::onButtonSensor(elev_button_type_t type, int floor)
{
    elevator->setButtonLamp(type, floor, 1);
    elevator->goToFloor(floor);

    call[type][floor] = true;
}
