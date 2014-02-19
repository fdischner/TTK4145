#include "elevator.h"
#include <QMetaType>


Elevator::Elevator(QObject *parent) :
    QThread(parent)
{
	qRegisterMetaType<elev_button_type_t>("elev_button_type_t");
	elev_init();
	connect(this, SIGNAL(floor_sensor(int)), this, SLOT(set_floor_indicator(int)));
	connect(this, SIGNAL(button_sensor(elev_button_type_t, int)), this, SLOT(onButtonSensor(elev_button_type_t, int)));
}

void Elevator::run() {
    set_speed(300);
      
    while (1) {
    	int tmp;

    	tmp = elev_get_floor_sensor_signal();
        if (tmp != -1)
        	emit floor_sensor(tmp);

		for (int i = 0; i < N_FLOORS; i++)
		{
			if (i != N_FLOORS-1 && elev_get_button_signal(BUTTON_CALL_UP, i))
				emit button_sensor(BUTTON_CALL_UP, i);
			if (i != 0 && elev_get_button_signal(BUTTON_CALL_DOWN, i))
				emit button_sensor(BUTTON_CALL_DOWN, i);
			if (elev_get_button_signal(BUTTON_COMMAND, i))
				emit button_sensor(BUTTON_COMMAND, i);
		}

        if (tmp == 0){
            set_speed(300);
        } else if (tmp == N_FLOORS-1){
            set_speed(-300);
        }

        if (elev_get_stop_signal()) {
        	emit stop_sensor();
            set_speed(0);
            break;
        }

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

