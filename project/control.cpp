// This class manages the control functionality of the elevator

#include "control.h"
#include "elevator.h"
#include "unistd.h"
#include "networkmanager.h"
#include <QDataStream>
#include <QTimer>
#include <QDateTime>
#include <QNetworkInterface>
#include <QtDebug>

// Time for the door to be open when servicing a floor
static const int SERVICE_TIME = 2000;
// Delay before an elevator starts moving
static const int IDLE_DELAY_TIME = 500;

Control::Control(const QByteArray &elev_state, QObject *parent) :
    QObject(parent), state()
{
    // Open a socket for sending messages to backup process
    local_network = new NetworkManager(this);
    local_network->initSocket(QAbstractSocket::UdpSocket, "127.0.0.1", 44445);

    // Open a socket for sending messages to other elevators
    elevator_network = new NetworkManager(this);
    elevator_network->initSocket(QAbstractSocket::UdpSocket, "129.241.187.255", 34444);
    connect(elevator_network, SIGNAL(messageReceived(QByteArray,QHostAddress)), this, SLOT(onMessageReceived(QByteArray,QHostAddress)));

    elevator = new Elevator(this);

    // Initialize the state of the elevator
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
    // NOTE: don't we need a default initialization?
    // No, c++ does that for us


    // Connect elevator signals to slots for handling requests and start running the elevator
    connect(elevator, SIGNAL(floorSensor(int)), this, SLOT(onFloorSensor(int)));
    connect(elevator, SIGNAL(buttonSensor(elev_button_type_t,int)), this, SLOT(onButtonSensor(elev_button_type_t,int)));
    elevator->start();

    // Start a timer to send imAlive messages every 100ms
    imAlive_timer = new QTimer(this);
    connect(imAlive_timer, SIGNAL(timeout()), this, SLOT(onSendMessage()));
    imAlive_timer->start(100);

    // Create a timer to send servicing floor messages
    service_timer = new QTimer(this);
    connect(service_timer, SIGNAL(timeout()), this, SLOT(onServiceTimer()));
}

void Control::onSendMessage() {
    // Save the current state of the elevator
    state.direction = elevator->direction;
    QByteArray elev_state = state.serialize();

    // Send elevator state (imAlive message) to backup
    local_network->sendMessage(elev_state);

    // Send elevator state to other elevators
    elevator_network->sendMessage(elev_state);
}

void Control::onMessageReceived(const QByteArray &message, const QHostAddress &sender) {
    elevator_state elev_state;

    if (elev_state.deserialize(message))
    {
        // Update the internal state of sending elevator
        for (int i = 0; i < N_FLOORS; i++)
        {
            // check if the state is newer than the one we have currently stored
            if (state.remote[sender.toIPv4Address()].call[i].first < elev_state.call[BUTTON_COMMAND][i].first)
                state.remote[sender.toIPv4Address()].call[i] = elev_state.call[BUTTON_COMMAND][i];
        }


        // Update external calls
        for (int i = 0; i < BUTTON_COMMAND; i++) {
            for (int j = 0; j < N_FLOORS; j++) {
                // check if the calls are newer than the ones we have currently stored
                if (elev_state.call[i][j].first > state.call[i][j].first)
                {
                    // update requests
                    state.call[i][j] = elev_state.call[i][j];
                    // update lamps
                    if (i == 0)
                        elevator->setButtonLamp(BUTTON_CALL_UP, j, state.call[i][j].second);
                    else
                        elevator->setButtonLamp(BUTTON_CALL_DOWN, j, state.call[i][j].second);
                }
            }
        }


        // Update our internal state
        foreach (const QHostAddress &address, QNetworkInterface::allAddresses())
        {
            if (elev_state.remote.contains(address.toIPv4Address()))
            {
                struct internal_state istate = elev_state.remote[address.toIPv4Address()];

                for (int i = 0; i < N_FLOORS; i++)
                {
                    // check if the internal state is newer than the one we have currently stored
                    if (istate.call[i].first > state.call[BUTTON_COMMAND][i].first)
                    {
                        // update requests
                        state.call[BUTTON_COMMAND][i] = istate.call[i];
                        // update lamps
                        elevator->setButtonLamp(BUTTON_COMMAND, i, istate.call[i].second);
                    }
                }

                // don't check other addresses, if we already found a match
                break;
            }
        }

        // If we're idle, then check for new calls to service
        QTimer::singleShot(IDLE_DELAY_TIME, this, SLOT(idleCheckCalls()));
    }
}

void Control::idleCheckCalls()
{
    // If elevator is busy, return
    if (service_timer->isActive() || elevator->moving)
        return;

    int i, j;

    // First, check for calls at the current floor and just service them
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
        // If there were no calls at the current floor, check others in order of proximity
        // then send the elevator to the appropriate floor
        // Note: internal calls have priority
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
    // First, check if there is a call up at the current floor and service it
    if (state.call[BUTTON_CALL_UP][floor].second)
    {
        serviceFloor(BUTTON_CALL_UP, floor);
        return true;
    }

    // Then, check calls going up
    for (int i = floor+1; i < N_FLOORS; i++)
    {
        if (state.call[BUTTON_COMMAND][i].second || state.call[BUTTON_CALL_UP][i].second)
        {
            elevator->goToFloor(i);
            return true;
        }
    }

    // Afterwards, check calls going down
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
    // First, check if there is a call down at the current floor and service it
    if (state.call[BUTTON_CALL_DOWN][floor].second)
    {
        serviceFloor(BUTTON_CALL_DOWN, floor);
        return true;
    }

    // Then, check calls going down
    for (int i = floor-1; i >=0; i--)
    {
        if (state.call[BUTTON_COMMAND][i].second || state.call[BUTTON_CALL_DOWN][i].second)
        {
            elevator->goToFloor(i);
            return true;
        }
    }

    // Afterwards, check calls going up
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
    // Internal calls are always serviced
    elevator->setButtonLamp(BUTTON_COMMAND, floor, false);
    state.call[BUTTON_COMMAND][floor].first = QDateTime::currentMSecsSinceEpoch() + SERVICE_TIME;
    state.call[BUTTON_COMMAND][floor].second = false;

    // If there is an external call, service it too
    // Note: set timestamp to end of service time, so button presses during service are ignored
    elevator->setButtonLamp(type, floor, false);
    state.call[type][floor].first = QDateTime::currentMSecsSinceEpoch() + SERVICE_TIME;
    state.call[type][floor].second = false;

    state.button_type = type;

    // Set the open door lamp
    elevator->setDoorOpenLamp(1);

    // Start the service timer
    service_timer->start(SERVICE_TIME);
}

void Control::onServiceTimer() {
    // Stop the service timer and turn off the lamp
    service_timer->stop();
    elevator->setDoorOpenLamp(0);

    // After servicing the floor, check if there are any other calls
    if (elevator->direction == UP)
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
    // Going up and there is a call up
    if (elevator->direction == UP && state.call[BUTTON_CALL_UP][floor].second)
        return true;

    // Going down and there is a call down
    if (elevator->direction == DOWN && state.call[BUTTON_CALL_DOWN][floor].second)
        return true;

    // Going up and there are no calls above, but there is call down
    if (elevator->direction == UP && !checkCallsAbove(floor) && state.call[BUTTON_CALL_DOWN][floor].second)
    {
        // change direction
        elevator->direction = DOWN;
        return true;
    }

    // Going down and there are not calls below, but there is call up
    if (elevator->direction == DOWN && !checkCallsBelow(floor) && state.call[BUTTON_CALL_UP][floor].second)
    {
        // change direction
        elevator->direction = UP;
        return true;
    }

    // There is an internal call
    if (state.call[BUTTON_COMMAND][floor].second)
        return true;

    // If there are no more requests, stop the elevator
    if (elevator->direction == DOWN && !checkCallsBelow(floor) && !checkCallsAbove(floor))
        elevator->stop();
    else if (elevator->direction == UP && !checkCallsAbove(floor) && !checkCallsBelow(floor))
        elevator->stop();

    // default case is to not service
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

        // Service the floor
        if (elevator->direction == UP)
            serviceFloor(BUTTON_CALL_UP, floor);
        else
            serviceFloor(BUTTON_CALL_DOWN, floor);
    }
}

void Control::onButtonSensor(elev_button_type_t type, int floor)
{
    // Ignore the request we're currently servicing
    if (service_timer->isActive() && floor == this->floor && type == state.button_type)
        return;

    // Set the lamp of the button pressed and save the request in the state
    elevator->setButtonLamp(type, floor, true);
    state.call[type][floor].first = QDateTime::currentMSecsSinceEpoch();
    state.call[type][floor].second = true;

    // If internal call, then check immediately
    if (type == BUTTON_COMMAND)
        idleCheckCalls();
    // otherwise, wait short delay in case another elevator is idle at that floor
    else
        QTimer::singleShot(IDLE_DELAY_TIME, this, SLOT(idleCheckCalls()));
}
