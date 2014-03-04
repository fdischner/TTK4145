#ifndef CONTROL_H
#define CONTROL_H

#include <QObject>
#include "elevator.h"

class Control : public QObject
{
    Q_OBJECT
public:
    explicit Control(QObject *parent = 0);

signals:

public slots:
    void onFloorSensor(int floor);
    void onButtonSensor(elev_button_type_t type, int floor);

private:
    Elevator *elevator;
    bool call[3][N_FLOORS];
};

#endif // CONTROL_H
