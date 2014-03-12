#include "control.h"
#include "elevator.h"
#include "unistd.h"
#include <QDataStream>

QByteArray elevator_state::serialize()
{
    QByteArray elev_state;
    QDataStream stream(&elev_state, QIODevice::WriteOnly);

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < N_FLOORS; j++) {
            stream << call[i][j];
        }
    }
    return elev_state;
}

bool elevator_state::deserialize(const QByteArray &state)
{
    if (state.size() == 0) {
        return false;
    }

    QDataStream stream(state);

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < N_FLOORS; j++) {
            stream >> call[i][j];
        }
    }

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

    //Initialize the state of the elevator
    state.deserialize(elev_state);

    elevator = new Elevator(this);

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
    QByteArray elev_state = state.serialize();

    // Send alive message (elevator state) to backup
    local_network->sendMessage(elev_state);
    elevator_network->sendMessage(elev_state);
}

void Control::onMessageReceived(const QByteArray &message) {
    elevator_state elev_state;
    elev_state.deserialize(message);

    //Restart timer
    message_timer->start();

    for (int i = 0; i < BUTTON_COMMAND; i++) {
        for (int j = 0; j < N_FLOORS; j++) {
            if (elev_state.call[i][j])
                state.call[i][j] = true;
        }
    }
}

bool Control::checkCallsAbove(int floor)
{
    // first check calls going up
    for (int i = floor+1; i < N_FLOORS; i++)
    {
        if (call[BUTTON_COMMAND][i] || call[BUTTON_CALL_UP][i])
        {
            elevator->goToFloor(i);
            return true;
        }
    }
    // then check calls going down
    for (int i = floor+1; i < N_FLOORS; i++)
    {
        if (call[BUTTON_COMMAND][i] || call[BUTTON_CALL_DOWN][i])
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
        if (call[BUTTON_COMMAND][i] || call[BUTTON_CALL_DOWN][i])
        {
            elevator->goToFloor(i);
            return true;
        }
    }
    // then check calls going up
    for (int i = floor-1; i >= 0; i--)
    {
        if (call[BUTTON_COMMAND][i] || call[BUTTON_CALL_UP][i])
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
    call[BUTTON_COMMAND][floor] = false;

    // if there is an external call, service it too
    call[type][floor] = false;
    elevator->setButtonLamp(type, floor, 0);

    elevator->setDoorOpenLamp(1);
    // FIXME: change this to a timer so the alive messages don't timeout
    //usleep(500000);
    // Start the service timer
    service_timer_cnt = 0;
    service_timer->start(100);
}

void Control::onServiceTimer() {
    service_timer_cnt++;

    //send message for floor being serviced

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

    if (elevator->direction == 1 && call[BUTTON_CALL_UP][floor])
        return true;

    if (elevator->direction == -1 && call[BUTTON_CALL_DOWN][floor])
        return true;

    if (elevator->direction == 1 && !checkCallsAbove(floor) && call[BUTTON_CALL_DOWN][floor])
    {
        elevator->direction = -1;
        return true;
    }

    if (elevator->direction == -1 && !checkCallsBelow(floor) && call[BUTTON_CALL_UP][floor])
    {
        elevator->direction = 1;
        return true;
    }

    if (call[BUTTON_COMMAND][floor])
        return true;

    // for safety
    if (floor == 0 || floor == N_FLOORS-1)
        return true;

    // In case there are no more requests, stop the elevator
    for (i = 0; i < N_FLOORS; i++) {
        if (call[0][i] || call[1][i] || call[2][i])
            break;
    }

    if (i == N_FLOORS)
        return true;

    return false;
}

void Control::onFloorSensor(int floor)
{
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

    call[type][floor] = true;

    if (!elevator->moving)
    {
        if (elevator->floor == floor)
            serviceFloor(type, floor);
        else
            elevator->goToFloor(floor);
    }
}
