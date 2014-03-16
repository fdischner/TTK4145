#include "elevator.h"
#include <QMetaType>

static const int SPEED = 100;

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
    elev_set_speed(0);
    // don't set the floor variable here so that the signal is emitted from the thread
}

void Elevator::run() {
    int prev_floor = -1;
    int prev_up = 0;
    int prev_down = 0;
    int prev_int = 0;
    bool prev_stop = false;
    bool prev_obs = false;

    if (elev_get_floor_sensor_signal() == -1)
    {
        moving = true;
        elev_set_speed(SPEED*direction);
    }

    while (1) {
        int cur, up, down, tmp;

        cur = elev_get_floor_sensor_signal();
        if (prev_floor != cur && cur != -1)
        {
            floor = cur;
            emit floorSensor(floor);
        }
        prev_floor = cur;

        cur = up = down = 0;
        // read button sensors
		for (int i = 0; i < N_FLOORS; i++)
		{
            // call up only for floors 0-2
			if (i != N_FLOORS-1 && elev_get_button_signal(BUTTON_CALL_UP, i))
                up |= (1 << i);
            // call down only for floors 1-3
			if (i != 0 && elev_get_button_signal(BUTTON_CALL_DOWN, i))
                down |= (1 << i);
			if (elev_get_button_signal(BUTTON_COMMAND, i))
                cur |= (1 << i);
        }

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

        // read stop sensor
        if (elev_get_stop_signal()) {
            if (!prev_stop)
                emit stopSensor();
            prev_stop = true;
        }
        else
        {
            prev_stop = false;
        }

        // read obstruction sensor
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

void Elevator::setFloorIndicator(int floor)
{
    if (floor >= 0 && floor < N_FLOORS)
        elev_set_floor_indicator(floor);

    if (wanted != -1 && floor == wanted)
        stop();

    // for safety
    if (floor == 0 || floor == N_FLOORS-1)
        stop();
}

void Elevator::setButtonLamp(elev_button_type_t button, int floor, int value)
{
    if (button == BUTTON_CALL_DOWN && floor == 0)
        return;

    if (button == BUTTON_CALL_UP && floor == N_FLOORS-1)
        return;

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

    elev_set_speed(direction * SPEED);
    moving = true;
}

void Elevator::stop()
{
    elev_set_speed(0);
    moving = false;
}
