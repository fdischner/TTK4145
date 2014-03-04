#include "elevator.h"
#include <QMetaType>


Elevator::Elevator(QObject *parent) :
    QThread(parent), floor(-1), wanted(-1), direction(-1), moving(false)
{
    // register elev_button_type_t so we can use it in signals/slots
	qRegisterMetaType<elev_button_type_t>("elev_button_type_t");

    // initialize the elevator driver
	elev_init();

    // set the floor indicator light when the sensor is detected
    connect(this, SIGNAL(floorSensor(int)), this, SLOT(setFloorIndicator(int)));

    // reset elevator to a known position
    direction = -1;
    elev_set_speed(-300);
    while(elev_get_floor_sensor_signal() == -1);
    // don't set the floor variable here so that the signal is emitted from the thread
    elev_set_speed(0);
}

void Elevator::run() {
    while (1) {
    	int tmp;

    	tmp = elev_get_floor_sensor_signal();
        if (tmp != -1 && tmp != floor)
        {
            floor = tmp;
            emit floorSensor(floor);
        }

        // read button sensors
		for (int i = 0; i < N_FLOORS; i++)
		{
            // call up only for floors 0-2
			if (i != N_FLOORS-1 && elev_get_button_signal(BUTTON_CALL_UP, i))
                emit buttonSensor(BUTTON_CALL_UP, i);
            // call down only for floors 1-3
			if (i != 0 && elev_get_button_signal(BUTTON_CALL_DOWN, i))
                emit buttonSensor(BUTTON_CALL_DOWN, i);
			if (elev_get_button_signal(BUTTON_COMMAND, i))
                emit buttonSensor(BUTTON_COMMAND, i);
		}

        // read stop sensor
        if (elev_get_stop_signal()) {
            emit stopSensor();
        }

        // read obstruction sensor
		if (elev_get_obstruction_signal())
		{
            emit obstructionSensor();
		}
    }
}

void Elevator::setFloorIndicator(int floor)
{
    if (floor >= 0 && floor < N_FLOORS)
        elev_set_floor_indicator(floor);

    if (wanted != -1 && floor == wanted)
        stop();
}

void Elevator::setButtonLamp(elev_button_type_t button, int floor, int value)
{
	elev_set_button_lamp(button, floor, value);
}

void Elevator::setStopLamp(int value)
{
	elev_set_stop_lamp(value);
}

void Elevator::setDoorOpenLamp(int value)
{
	elev_set_door_open_lamp(value);
}

void Elevator::goToFloor(int floor)
{
    wanted = floor;

    if (wanted < this->floor)
        direction = -1;
    else if (wanted > this->floor)
        direction = 1;
    else if (elev_get_floor_sensor_signal() == -1)
        direction = -direction;
    else
    {
        stop();
        return;
    }

    elev_set_speed(direction * 100);
}

void Elevator::stop()
{
    elev_set_speed(0);
}
