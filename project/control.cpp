#include "control.h"
#include "elevator.h"
#include "unistd.h"


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

bool Control::checkCallsAbove(int floor)
{
    // first check calls going up
    for (int i = floor+1; i < N_FLOORS; i++)
    {
        if (call[BUTTON_COMMAND][i] || call[BUTTON_CALL_UP][i])
        {
            elevator->goToFloor(i);
            return true;
        }
    }
    // then check calls going down
    for (int i = floor+1; i < N_FLOORS; i++)
    {
        if (call[BUTTON_COMMAND][i] || call[BUTTON_CALL_DOWN][i])
        {
            elevator->goToFloor(i);
            return true;
        }
    }

    return false;
}

bool Control::checkCallsBelow(int floor)
{
    // first check calls going down
    for (int i = floor-1; i >=0; i--)
    {
        if (call[BUTTON_COMMAND][i] || call[BUTTON_CALL_DOWN][i])
        {
            elevator->goToFloor(i);
            return true;
        }
    }
    // then check calls going up
    for (int i = floor-1; i >= 0; i--)
    {
        if (call[BUTTON_COMMAND][i] || call[BUTTON_CALL_UP][i])
        {
            elevator->goToFloor(i);
            return true;
        }
    }

    return false;
}

void Control::serviceFloor(elev_button_type_t type, int floor)
{
    // internal calls are always serviced
    elevator->setButtonLamp(BUTTON_COMMAND, floor, 0);
    call[BUTTON_COMMAND][floor] = false;

    // if there is an external call, service it too
    call[type][floor] = false;
    elevator->setButtonLamp(type, floor, 0);

    elevator->setDoorOpenLamp(1);
    sleep(1);
    elevator->setDoorOpenLamp(0);

    if (elevator->direction == 1)
    {
        if (!checkCallsAbove(floor))
            checkCallsBelow(floor);
    }
    else
    {
        if (!checkCallsBelow(floor))
            checkCallsAbove(floor);
    }
}

bool Control::shouldService(int floor)
{
    int i;

    if (elevator->direction == 1 && !checkCallsAbove(floor) && call[BUTTON_CALL_DOWN][floor])
    {
        elevator->direction = -1;
        return true;
    }

    if (elevator->direction == -1 && !checkCallsBelow(floor) && call[BUTTON_CALL_UP][floor])
    {
        elevator->direction = 1;
        return true;
    }

    if (call[BUTTON_COMMAND][floor])
        return true;

    if (elevator->direction == 1 && call[BUTTON_CALL_UP][floor])
        return true;

    if (elevator->direction == -1 && call[BUTTON_CALL_DOWN][floor])
        return true;

    // for safety
    if (floor == 0 || floor == N_FLOORS-1)
        return true;

    // In case there are no more requests, stop the elevator
    for (i = 0; i < N_FLOORS; i++) {
        if (call[0][i] || call[1][i] || call[2][i])
            break;
    }

    if (i == N_FLOORS)
        return true;

    return false;
}

void Control::onFloorSensor(int floor)
{
    if (shouldService(floor))
    {
        elevator->stop();

        if (elevator->direction == 1)
            serviceFloor(BUTTON_CALL_UP, floor);
        else
            serviceFloor(BUTTON_CALL_DOWN, floor);
    }
}

void Control::onButtonSensor(elev_button_type_t type, int floor)
{
    elevator->setButtonLamp(type, floor, 1);

    call[type][floor] = true;

    if (!elevator->moving)
    {
        if (elevator->floor == floor)
            serviceFloor(type, floor);
        else
            elevator->goToFloor(floor);
    }
}
