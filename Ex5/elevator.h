#ifndef ELEVATOR_H
#define ELEVATOR_H

#include <QThread>
extern "C" {
#include "elev.h"
}

class Elevator : public QThread
{
    Q_OBJECT

public:
    Elevator(QObject *parent = 0);

protected:
	void run();

private:
	
signals:
	void floor_sensor(int);
	void button_sensor(elev_button_type_t, int);
	void stop_sensor(void);
	void obstruction_sensor(void);
	
public slots:
	void set_speed(int speed);
	void set_floor_indicator(int floor);
	void set_button_lamp(elev_button_type_t button, int floor, int value);
	void set_stop_lamp(int value);
	void set_door_open_lamp(int value);
	
private slots:
	void onButtonSensor(elev_button_type_t button, int floor);
};

#endif
