// This class is executed as a thread to receive signals from
// button pushes and floor sensors and control the elevator

#include "elevator.h"
#include <QMetaType>

// Elevator's speed
static const int SPEED = 100;

Elevator::Elevator(QObject *parent) :
    QThread(parent), floor(-1), wanted(-1), direction(DOWN), moving(false)
{
    // Register elev_button_type_t so we can use it in signals/slots
	qRegisterMetaType<elev_button_type_t>("elev_button_type_t");

    // Initialize the elevator driver
	elev_init();
    elev_set_speed(0);

    // Set the floor indicator light when the sensor is detected
    connect(this, SIGNAL(floorSensor(int)), this, SLOT(onFloorSensor(int)));

    // Don't set the floor variable here so that the signal is emitted from the thread
}

void Elevator::run() {
    int prev_floor = -1;
    int prev_up = 0;
    int prev_down = 0;
    int prev_int = 0;
    bool prev_stop = false;
    bool prev_obs = false;

    // If elevator is between floors, move until it reaches one
    if (elev_get_floor_sensor_signal() == -1)
    {
        moving = true;
        elev_set_speed(SPEED*direction);
    }

    while (1) {
        int cur, up, down, tmp;

        // Read the floor sensors
        cur = elev_get_floor_sensor_signal();

        // when it gets to a new floor, emit a signal
        if (prev_floor != cur && cur != -1)
        {
            floor = cur;
            emit floorSensor(floor);
        }
        prev_floor = cur;


        // Read the button sensors
        cur = up = down = 0;

		for (int i = 0; i < N_FLOORS; i++)
		{
            // check calls up only for floors 0-2 and save them
			if (i != N_FLOORS-1 && elev_get_button_signal(BUTTON_CALL_UP, i))
                up |= (1 << i);
            // check calls down only for floors 1-3 and save them
			if (i != 0 && elev_get_button_signal(BUTTON_CALL_DOWN, i))
                down |= (1 << i);
            // check internal calls for every floor and save them
			if (elev_get_button_signal(BUTTON_COMMAND, i))
                cur |= (1 << i);
        }

        // emit a signal for every button pressed
        tmp = cur & (cur ^ prev_int);
        for (int i = 0; i < N_FLOORS; i++)
        {
            if (tmp & (1 << i))
                emit buttonSensor(BUTTON_COMMAND, i);
        }
        tmp = up & (up ^ prev_up);
        for (int i = 0; i < N_FLOORS; i++)
        {
            if (tmp & (1 << i))
                emit buttonSensor(BUTTON_CALL_UP, i);
        }
        tmp = down & (down ^ prev_down);
        for (int i = 0; i < N_FLOORS; i++)
        {
            if (tmp & (1 << i))
                emit buttonSensor(BUTTON_CALL_DOWN, i);
        }
        prev_int = cur;
        prev_down = down;
        prev_up = up;


        // Read the stop sensor
        if (elev_get_stop_signal()) {
            if (!prev_stop)
                emit stopSensor();
            prev_stop = true;
        }
        else
        {
            prev_stop = false;
        }


        // Read the obstruction sensor
		if (elev_get_obstruction_signal())
		{
            if (!prev_obs)
                emit obstructionSensor();
            prev_obs = true;
		}
        else
        {
            prev_obs = false;
        }
    }
}

void Elevator::onFloorSensor(int floor)
{
    // Set on the lamp of the current floor
    if (floor >= 0 && floor < N_FLOORS)
        elev_set_floor_indicator(floor);

    // stop elevator if this is the requested floor
    if (wanted != -1 && floor == wanted)
        stop();

    // For safety, always stop the elevator at the first or last floor
    if (floor == 0 || floor == N_FLOORS-1)
        stop();
}

void Elevator::setButtonLamp(elev_button_type_t button, int floor, bool value)
{
    // Check if the button exists on the floor
    if (button == BUTTON_CALL_DOWN && floor == 0)
        return;
    if (button == BUTTON_CALL_UP && floor == N_FLOORS-1)
        return;

    // Set on/off the lamp of the button pressed
    elev_set_button_lamp(button, floor, value ? 1 : 0);
}

void Elevator::setStopLamp(int value)
{
    // Set on/off the stop lamp
	elev_set_stop_lamp(value);
}

void Elevator::setDoorOpenLamp(int value)
{
    // Set on/off the open door lamp
	elev_set_door_open_lamp(value);
}

void Elevator::goToFloor(int floor)
{
    wanted = floor;

    // Set the direction of the elevator
    if (wanted < this->floor)
        direction = DOWN;
    else if (wanted > this->floor)
        direction = UP;
    else if (elev_get_floor_sensor_signal() == -1)
        // if last sensor was the requested floor, but we've
        // already passed it, then reverse direction
        direction = -direction;
    else
    {
        stop();
        return;
    }

    // Start moving the elevator
    elev_set_speed(direction * SPEED);
    moving = true;
}

void Elevator::stop()
{
    // Stop the elevator
    elev_set_speed(0);
    moving = false;
}
