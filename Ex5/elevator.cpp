#include "elevator.h"
#include <QMetaType>


Elevator::Elevator(QObject *parent) :
    QThread(parent), floor(-1)
{
    // register elev_button_type_t so we can use it in signals/slots
	qRegisterMetaType<elev_button_type_t>("elev_button_type_t");

    // initialize the elevator driver
	elev_init();

    // set the floor indicator light when the sensor is detected
	connect(this, SIGNAL(floor_sensor(int)), this, SLOT(set_floor_indicator(int)));
    // light the buttons when they're pressed
	connect(this, SIGNAL(button_sensor(elev_button_type_t, int)), this, SLOT(onButtonSensor(elev_button_type_t, int)));

    // reset elevator to a known position
    set_speed(-300);
    while(elev_get_floor_sensor_signal() == -1);
    // don't set the floor variable here so that the signal is emitted from the thread
    set_speed(0);
}

void Elevator::run() {
    while (1) {
    	int tmp;

    	tmp = elev_get_floor_sensor_signal();
        if (tmp != -1 && tmp != floor)
        {
            floor = tmp;
        	emit floor_sensor(floor);
        }

        // read button sensors
		for (int i = 0; i < N_FLOORS; i++)
		{
            // call up only for floors 0-2
			if (i != N_FLOORS-1 && elev_get_button_signal(BUTTON_CALL_UP, i))
				emit button_sensor(BUTTON_CALL_UP, i);
            // call down only for floors 1-3
			if (i != 0 && elev_get_button_signal(BUTTON_CALL_DOWN, i))
				emit button_sensor(BUTTON_CALL_DOWN, i);
			if (elev_get_button_signal(BUTTON_COMMAND, i))
				emit button_sensor(BUTTON_COMMAND, i);
		}

        // read stop sensor
        if (elev_get_stop_signal()) {
        	emit stop_sensor();
            set_speed(0);
        }

        // read obstruction sensor
		if (elev_get_obstruction_signal())
		{
			emit obstruction_sensor();
		}
    }
}

void Elevator::set_speed(int speed)
{
	elev_set_speed(speed);
}

void Elevator::set_floor_indicator(int floor)
{
    if (floor >= 0 && floor < N_FLOORS)
        elev_set_floor_indicator(floor);
}

void Elevator::set_button_lamp(elev_button_type_t button, int floor, int value)
{
	elev_set_button_lamp(button, floor, value);
}

void Elevator::set_stop_lamp(int value)
{
	elev_set_stop_lamp(value);
}

void Elevator::set_door_open_lamp(int value)
{
	elev_set_door_open_lamp(value);
}

void Elevator::onButtonSensor(elev_button_type_t button, int floor)
{
	set_button_lamp(button, floor, 1);
}

