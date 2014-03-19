#include "control.h"
#include "elevator.h"
#include "unistd.h"
#include <QDataStream>
#include "networkmanager.h"
#include <QTimer>
#include <QDateTime>
#include <QNetworkInterface>
#include <QtDebug>

static const int SERVICE_TIME = 2000;
static const int IDLE_DELAY_TIME = 500;

Control::Control(const QByteArray &elev_state, QObject *parent) :
    QObject(parent), state()
{
    //Open a socket for sending messages to backup process
    local_network = new NetworkManager(this);
    local_network->initSocket(QAbstractSocket::UdpSocket, "127.0.0.1", 44445);

    //Open a socket for sending messages to other elevators
    elevator_network = new NetworkManager(this);
    elevator_network->initSocket(QAbstractSocket::UdpSocket, "129.241.187.255", 34444);
    connect(elevator_network, SIGNAL(messageReceived(QByteArray,QHostAddress)), this, SLOT(onMessageReceived(QByteArray,QHostAddress)));

    elevator = new Elevator(this);

    //Initialize the state of the elevator
    if (state.deserialize(elev_state))
    {
        for (int j = 0; j < N_FLOORS; j++) {
            if (state.call[BUTTON_CALL_UP][j].second)
                elevator->setButtonLamp(BUTTON_CALL_UP, j, 1);
            if (state.call[BUTTON_CALL_DOWN][j].second)
                elevator->setButtonLamp(BUTTON_CALL_DOWN, j, 1);
            if (state.call[BUTTON_COMMAND][j].second)
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

void Control::onMessageReceived(const QByteArray &message, const QHostAddress &sender) {
    QDataStream stream(message);
    elevator_state elev_state;

    if (elev_state.deserialize(message))
    {
        // update internal state of sending elevator
        for (int i = 0; i < N_FLOORS; i++)
        {
            if (state.remote[sender.toIPv4Address()].call[i].first < elev_state.call[BUTTON_COMMAND][i].first)
                state.remote[sender.toIPv4Address()].call[i] = elev_state.call[BUTTON_COMMAND][i];
        }

        // update external calls
        for (int i = 0; i < BUTTON_COMMAND; i++) {
            for (int j = 0; j < N_FLOORS; j++) {
                // update only if newer
                if (elev_state.call[i][j].first > state.call[i][j].first)
                {
                    // set new state
                    state.call[i][j] = elev_state.call[i][j];
                    // update lamp
                    if (i == 0)
                        elevator->setButtonLamp(BUTTON_CALL_UP, j, state.call[i][j].second);
                    else
                        elevator->setButtonLamp(BUTTON_CALL_DOWN, j, state.call[i][j].second);
                }
            }
        }

        // update our internal state, if received is newer
        foreach (const QHostAddress &address, QNetworkInterface::allAddresses())
        {
            if (elev_state.remote.contains(address.toIPv4Address()))
            {
                struct internal_state istate = elev_state.remote[address.toIPv4Address()];

                for (int i = 0; i < N_FLOORS; i++)
                {
                    if (istate.call[i].first > state.call[BUTTON_COMMAND][i].first)
                    {
                        state.call[BUTTON_COMMAND][i] = istate.call[i];
                        elevator->setButtonLamp(BUTTON_COMMAND, i, istate.call[i].second);
                    }
                }

                // don't check other addresses, if we already found a match
                break;
            }
        }

        // if we're idle, then check for new calls to service
        QTimer::singleShot(IDLE_DELAY_TIME, this, SLOT(idleCheckCalls()));
    }
}

void Control::idleCheckCalls()
{
    if (service_timer->isActive() || elevator->moving)
        return;

    int i, j;

    // first check for calls at the current floor and just service them
    if (state.call[BUTTON_CALL_DOWN][floor].second)
    {
        serviceFloor(BUTTON_CALL_DOWN, floor);
    }
    else if (state.call[BUTTON_CALL_UP][floor].second)
    {
        serviceFloor(BUTTON_CALL_UP, floor);
    }
    else if (state.call[BUTTON_COMMAND][floor].second)
    {
        serviceFloor(BUTTON_COMMAND, floor);
    }
    else
    {
        // if there were no calls at the current floor, check others in order of proximity
        // then send the elevator to the appropriate floor
        // internal calls have priority
        // TODO: make this a function
        for (i = floor-1, j = floor+1; i >= 0 || j < N_FLOORS; i--, j++)
        {
            if (i >= 0)
            {
                if (state.call[BUTTON_COMMAND][i].second)
                {
                    elevator->goToFloor(i);
                    return;
                }
            }
            if (j < N_FLOORS)
            {
                if (state.call[BUTTON_COMMAND][j].second)
                {
                    elevator->goToFloor(j);
                    return;
                }
            }
        }

        for (i = floor-1, j = floor+1; i >= 0 || j < N_FLOORS; i--, j++)
        {
            if (i >= 0)
            {
                if (state.call[BUTTON_CALL_DOWN][i].second || state.call[BUTTON_CALL_UP][i].second)
                {
                    elevator->goToFloor(i);
                    return;
                }
            }
            if (j < N_FLOORS)
            {
                if (state.call[BUTTON_CALL_DOWN][j].second || state.call[BUTTON_CALL_UP][j].second)
                {
                    elevator->goToFloor(j);
                    return;
                }
            }
        }
    }
}

bool Control::checkCallsAbove(int floor)
{
    if (state.call[BUTTON_CALL_UP][floor].second)
    {
        serviceFloor(BUTTON_CALL_UP, floor);
        return true;
    }

    // first check calls going up
    for (int i = floor+1; i < N_FLOORS; i++)
    {
        if (state.call[BUTTON_COMMAND][i].second || state.call[BUTTON_CALL_UP][i].second)
        {
            elevator->goToFloor(i);
            return true;
        }
    }
    // then check calls going down
    for (int i = floor+1; i < N_FLOORS; i++)
    {
        if (state.call[BUTTON_COMMAND][i].second || state.call[BUTTON_CALL_DOWN][i].second)
        {
            elevator->goToFloor(i);
            return true;
        }
    }

    return false;
}

bool Control::checkCallsBelow(int floor)
{
    if (state.call[BUTTON_CALL_DOWN][floor].second)
    {
        serviceFloor(BUTTON_CALL_DOWN, floor);
        return true;
    }

    // first check calls going down
    for (int i = floor-1; i >=0; i--)
    {
        if (state.call[BUTTON_COMMAND][i].second || state.call[BUTTON_CALL_DOWN][i].second)
        {
            elevator->goToFloor(i);
            return true;
        }
    }
    // then check calls going up
    for (int i = floor-1; i >= 0; i--)
    {
        if (state.call[BUTTON_COMMAND][i].second || state.call[BUTTON_CALL_UP][i].second)
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
    elevator->setButtonLamp(BUTTON_COMMAND, floor, false);
    state.call[BUTTON_COMMAND][floor].first = QDateTime::currentMSecsSinceEpoch();
    state.call[BUTTON_COMMAND][floor].second = false;

    // if there is an external call, service it too
    // set timestamp to end of service time so button presses during service are ignored
    state.call[type][floor].first = QDateTime::currentMSecsSinceEpoch() + SERVICE_TIME;
    state.call[type][floor].second = false;
    elevator->setButtonLamp(type, floor, false);

    state.button_type = type;

    elevator->setDoorOpenLamp(1);

    // Start the service timer
    service_timer->start(SERVICE_TIME);
}

void Control::onServiceTimer() {
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

    if (elevator->direction == 1 && state.call[BUTTON_CALL_UP][floor].second)
        return true;

    if (elevator->direction == -1 && state.call[BUTTON_CALL_DOWN][floor].second)
        return true;

    if (elevator->direction == 1 && !checkCallsAbove(floor) && state.call[BUTTON_CALL_DOWN][floor].second)
    {
        elevator->direction = -1;
        return true;
    }

    if (elevator->direction == -1 && !checkCallsBelow(floor) && state.call[BUTTON_CALL_UP][floor].second)
    {
        elevator->direction = 1;
        return true;
    }

    if (state.call[BUTTON_COMMAND][floor].second)
        return true;

    if (elevator->direction == -1 && !checkCallsBelow(floor) && checkCallsAbove(floor))
        return false;

    if (elevator->direction == 1 && !checkCallsAbove(floor) && checkCallsBelow(floor))
        return false;

    // In case there are no more requests, stop the elevator
    for (i = 0; i < N_FLOORS; i++) {
        if (state.call[0][i].second || state.call[1][i].second || state.call[2][i].second)
            break;
    }

    if (i == N_FLOORS)
    {
        // stop the elevator, but don't service
        elevator->stop();
    }

    return false;
}

void Control::onFloorSensor(int floor)
{
    // Update our floor
    this->floor = floor;

    // Check if the current floor should be serviced
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
    // ignore the request we're currently servicing
    if (service_timer->isActive() && floor == this->floor && type == state.button_type)
        return;

    elevator->setButtonLamp(type, floor, true);
    state.call[type][floor].first = QDateTime::currentMSecsSinceEpoch();
    state.call[type][floor].second = true;

    // if internal call, then check immediately
    if (type == BUTTON_COMMAND)
        idleCheckCalls();
    // otherwise, wait short delay in case another elevator is idle at that floor
    else
        QTimer::singleShot(IDLE_DELAY_TIME, this, SLOT(idleCheckCalls()));
}
