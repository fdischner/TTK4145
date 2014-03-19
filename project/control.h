#ifndef CONTROL_H
#define CONTROL_H

#include <QObject>
#include "elevator.h"
#include <QDataStream>
#include <QHostAddress>
#include "state.h"

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
    NetworkManager *local_network;
    NetworkManager *elevator_network;
    QTimer *imAlive_timer, *service_timer;
    Elevator *elevator;
    elevator_state state;
    int service_timer_cnt;
    int floor;
};

#endif // CONTROL_H
