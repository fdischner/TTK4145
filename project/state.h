// This file contains the structures to hold the elevator's state

#ifndef STATE_H
#define STATE_H

#include "elev.h"
#include <QMap>
#include <QPair>
#include <QDataStream>

typedef QPair<qint64, bool> Button;

struct internal_state {
    Button call[N_FLOORS];
};

struct elevator_state {
  Button call[3][N_FLOORS];
  int direction;
  QMap<quint32, internal_state> remote;
  elev_button_type_t button_type;

  bool deserialize(const QByteArray &state);
  QByteArray serialize();
};

QDataStream &operator<<(QDataStream & stream, const internal_state &state);
QDataStream &operator>>(QDataStream & stream, internal_state &state);

//QDataStream &operator<<(QDataStream & stream, const elev_button_type_t &type);
//QDataStream &operator>>(QDataStream & stream, elev_button_type_t &type);

#endif
