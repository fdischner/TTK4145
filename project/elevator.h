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

signals:
    void floorSensor(int);
    void buttonSensor(elev_button_type_t, int);
    void stopSensor(void);
    void obstructionSensor(void);
	
public slots:
    void setFloorIndicator(int floor);
    void setButtonLamp(elev_button_type_t button, int floor, int value);
    void setStopLamp(int value);
    void setDoorOpenLamp(int value);
    void goToFloor(int floor);
    void stop(void);
	
public:
    int floor, wanted;
    int direction;
    bool moving;
};

#endif
