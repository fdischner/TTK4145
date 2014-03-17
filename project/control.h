#ifndef CONTROL_H
#define CONTROL_H

#include <QObject>
#include "elevator.h"
#include <QDataStream>
#include <QHostAddress>
#include <QMap>

class NetworkManager;
class QTimer;

struct internal_state {
    bool call[N_FLOORS];
    qint64 timestamp;
};

struct elevator_state {
  bool call[3][N_FLOORS];
  int direction;
  qint64 timestamp;
  QMap<quint32, internal_state> remote;

  elev_button_type_t button_type;

  bool deserialize(const QByteArray &state);
  QByteArray serialize();
};

QDataStream &operator<<(QDataStream & stream, const internal_state &state);
QDataStream &operator>>(QDataStream & stream, internal_state &state);

QDataStream &operator<<(QDataStream & stream, const elev_button_type_t &type);
QDataStream &operator>>(QDataStream & stream, elev_button_type_t &type);

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
