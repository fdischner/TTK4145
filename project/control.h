// This class manages the control functionality of the elevator

#ifndef CONTROL_H
#define CONTROL_H

#include "elevator.h"
#include "state.h"
#include <QObject>
#include <QDataStream>
#include <QHostAddress>

class NetworkManager;
class QTimer;

class Control : public QObject
{
    Q_OBJECT

public:
    explicit Control(const QByteArray &elev_state, QObject *parent = 0);

private:
    bool checkCallsAbove(int floor);
    bool checkCallsBelow(int floor);
    void serviceFloor(elev_button_type_t type, int floor);
    bool shouldService(int floor);
    bool checkCallsByProximity(elev_button_type_t button_type1, elev_button_type_t button_type2 = BUTTON_COMMAND);

signals:

public slots:
    void onFloorSensor(int floor);
    void onButtonSensor(elev_button_type_t type, int floor);

private slots:
    void onSendMessage();
    void onMessageReceived(const QByteArray &message, const QHostAddress &sender);
    void onServiceTimer();
    void idleCheckCalls();

private:
    NetworkManager *local_network, *elevator_network;
    QTimer *imAlive_timer, *service_timer;
    Elevator *elevator;
    elevator_state state;
    int service_timer_cnt;
};

#endif
