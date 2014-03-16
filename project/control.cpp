#include "control.h"
#include "elevator.h"
#include "unistd.h"
#include <QDataStream>
#include "networkmanager.h"
#include <QTimer>
#include <QtDebug>

enum {
    STATE,
    SERVICE,
};

QByteArray elevator_state::serialize()
{
    QByteArray elev_state;
    QDataStream stream(&elev_state, QIODevice::WriteOnly);
    qint8 type = STATE;

    stream << type;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < N_FLOORS; j++) {
            stream << call[i][j];
        }
    }
    stream << direction;
    return elev_state;
}

bool elevator_state::deserialize(const QByteArray &state)
{
    qint8 type;

    if (state.size() == 0) {
        return false;
    }

    QDataStream stream(state);

    stream >> type;
    if (type != STATE)
        return false;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < N_FLOORS; j++) {
            stream >> call[i][j];
        }
    }

    stream >> direction;
    return true;
}

Control::Control(const QByteArray &elev_state, QObject *parent) :
    QObject(parent), state()
{
    //Open a socket for sending messages to backup process
    local_network = new NetworkManager(this);
    local_network->initSocket(QAbstractSocket::UdpSocket, "127.0.0.1", 44445);

    //Open a socket for sending messages to other elevators
    elevator_network = new NetworkManager(this);
    elevator_network->initSocket(QAbstractSocket::UdpSocket, "129.241.187.255", 44444);
    connect(elevator_network, SIGNAL(messageReceived(QByteArray)), this, SLOT(onMessageReceived(QByteArray)));

    elevator = new Elevator(this);

    //Initialize the state of the elevator
    if (state.deserialize(elev_state))
    {
        for (int j = 0; j < N_FLOORS; j++) {
            if (state.call[BUTTON_CALL_UP][j])
                elevator->setButtonLamp(BUTTON_CALL_UP, j, 1);
            if (state.call[BUTTON_CALL_DOWN][j])
                elevator->setButtonLamp(BUTTON_CALL_DOWN, j, 1);
            if (state.call[BUTTON_COMMAND][j])
                elevator->setButtonLamp(BUTTON_COMMAND, j, 1);
        }

        elevator->direction = state.direction;
    }

    connect(elevator, SIGNAL(floorSensor(int)), this, SLOT(onFloorSensor(int)));
    connect(elevator, SIGNAL(buttonSensor(elev_button_type_t,int)), this, SLOT(onButtonSensor(elev_button_type_t,int)));

    elevator->start();

    //Start a timer to send imAlive messages
    imAlive_timer = new QTimer(this);
    connect(imAlive_timer, SIGNAL(timeout()), this, SLOT(onSendMessage()));
    // Send alive messages every 100ms
    imAlive_timer->start(100);

    //Start a timer to send servicing floor messages
    service_timer = new QTimer(this);
    connect(service_timer, SIGNAL(timeout()), this, SLOT(onServiceTimer()));
}

void Control::onSendMessage() {
    state.direction = elevator->direction;
    QByteArray elev_state = state.serialize();

    // Send alive message (elevator state) to backup
    local_network->sendMessage(elev_state);
    elevator_network->sendMessage(elev_state);
}

void Control::onMessageReceived(const QByteArray &message) {
    QDataStream stream(message);
    qint8 type;

    stream >> type;

    switch (type)
    {
    case STATE:
        elevator_state elev_state;
        if (elev_state.deserialize(message))
        {
            for (int i = 0; i < BUTTON_COMMAND; i++) {
                for (int j = 0; j < N_FLOORS; j++) {
                    // if we're already servicing this request,
                    // then ignore status of other elevators
                    if (j == floor &&
                        ((elevator->direction == 1 && i == BUTTON_CALL_UP) ||
                         (elevator->direction == -1 && i == BUTTON_CALL_DOWN)))
                        continue;

                    if (elev_state.call[i][j])
                    {
                        state.call[i][j] = true;
                        if (i == 0)
                            elevator->setButtonLamp(BUTTON_CALL_UP, j, 1);
                        else
                            elevator->setButtonLamp(BUTTON_CALL_DOWN, j, 1);
                    }
                }
            }
            // TODO: check if elevator should start moving
        }
        break;
    case SERVICE:
        int floor;
        int direction;
        stream >> floor;
        stream >> direction;
        if (direction == 1)
            state.call[BUTTON_CALL_UP][floor] = false;
        else if (direction == -1)
            state.call[BUTTON_CALL_DOWN][floor] = false;
        break;
    }



}

bool Control::checkCallsAbove(int floor)
{
    // first check calls going up
    for (int i = floor+1; i < N_FLOORS; i++)
    {
        if (state.call[BUTTON_COMMAND][i] || state.call[BUTTON_CALL_UP][i])
        {
            elevator->goToFloor(i);
            return true;
        }
    }
    // then check calls going down
    for (int i = floor+1; i < N_FLOORS; i++)
    {
        if (state.call[BUTTON_COMMAND][i] || state.call[BUTTON_CALL_DOWN][i])
        {
            elevator->goToFloor(i);
            return true;
        }
    }

    return false;
}

bool Control::checkCallsBelow(int floor)
{
    // first check calls going down
    for (int i = floor-1; i >=0; i--)
    {
        if (state.call[BUTTON_COMMAND][i] || state.call[BUTTON_CALL_DOWN][i])
        {
            elevator->goToFloor(i);
            return true;
        }
    }
    // then check calls going up
    for (int i = floor-1; i >= 0; i--)
    {
        if (state.call[BUTTON_COMMAND][i] || state.call[BUTTON_CALL_UP][i])
        {
            elevator->goToFloor(i);
            return true;
        }
    }

    return false;
}

void Control::serviceFloor(elev_button_type_t type, int floor)
{
    // internal calls are always serviced
    elevator->setButtonLamp(BUTTON_COMMAND, floor, 0);
    state.call[BUTTON_COMMAND][floor] = false;

    // if there is an external call, service it too
    state.call[type][floor] = false;
    elevator->setButtonLamp(type, floor, 0);

    elevator->setDoorOpenLamp(1);

    // Start the service timer
    service_timer_cnt = 0;
    service_timer->start(100);
}

void Control::onServiceTimer() {
    QByteArray message;
    qint8 type = SERVICE;
    QDataStream stream(&message, QIODevice::WriteOnly);

    service_timer_cnt++;

    //send message for floor being serviced
    stream << type;
    stream << floor;
    stream << state.direction;
    elevator_network->sendMessage(message);

    if (service_timer_cnt != 2000 / 100)
        return;

    service_timer->stop();
    elevator->setDoorOpenLamp(0);

    if (elevator->direction == 1)
    {
        if (!checkCallsAbove(floor))
            checkCallsBelow(floor);
    }
    else
    {
        if (!checkCallsBelow(floor))
            checkCallsAbove(floor);
    }
}

bool Control::shouldService(int floor)
{
    int i;

    if (elevator->direction == 1 && state.call[BUTTON_CALL_UP][floor])
        return true;

    if (elevator->direction == -1 && state.call[BUTTON_CALL_DOWN][floor])
        return true;

    if (elevator->direction == 1 && !checkCallsAbove(floor) && state.call[BUTTON_CALL_DOWN][floor])
    {
        elevator->direction = -1;
        return true;
    }

    if (elevator->direction == -1 && !checkCallsBelow(floor) && state.call[BUTTON_CALL_UP][floor])
    {
        elevator->direction = 1;
        return true;
    }

    if (state.call[BUTTON_COMMAND][floor])
        return true;

    if (elevator->direction == -1 && !checkCallsBelow(floor) && checkCallsAbove(floor))
        return false;

    if (elevator->direction == 1 && !checkCallsAbove(floor) && checkCallsBelow(floor))
        return false;

    // for safety
    if (floor == 0 || floor == N_FLOORS-1)
        return true;

    // In case there are no more requests, stop the elevator
    for (i = 0; i < N_FLOORS; i++) {
        if (state.call[0][i] || state.call[1][i] || state.call[2][i])
            break;
    }

    if (i == N_FLOORS)
        return true;

    return false;
}

void Control::onFloorSensor(int floor)
{
    this->floor = floor;
    if (shouldService(floor))
    {
        elevator->stop();

        if (elevator->direction == 1)
            serviceFloor(BUTTON_CALL_UP, floor);
        else
            serviceFloor(BUTTON_CALL_DOWN, floor);
    }
}

void Control::onButtonSensor(elev_button_type_t type, int floor)
{
    elevator->setButtonLamp(type, floor, 1);

    state.call[type][floor] = true;

    if (service_timer->isActive())
        return;

    if (!elevator->moving)
    {
        if (elevator->floor == floor)
            serviceFloor(type, floor);
        else
            elevator->goToFloor(floor);
    }
}
