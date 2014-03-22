// This class is executed as a thread to receive signals from
// button pushes and floor sensors and control the elevator

#ifndef ELEVATOR_H
#define ELEVATOR_H

#include <QThread>
extern "C" {
#include "elev.h"
}

#define UP 1
#define DOWN -1

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
    void setButtonLamp(elev_button_type_t button, int floor, bool value);
    void setStopLamp(int value);
    void setDoorOpenLamp(int value);
    void goToFloor(int floor);
    void stop(void);

private slots:
    void onFloorSensor(int floor);
	
public:
    int floor, wanted;
    int direction;
    bool moving;
};

#endif
