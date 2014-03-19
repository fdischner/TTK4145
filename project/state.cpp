// This file contains the structures to hold the elevator's state

#include "state.h"

QByteArray elevator_state::serialize()
{
    QByteArray elev_state;
    QDataStream stream(&elev_state, QIODevice::WriteOnly);

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < N_FLOORS; j++) {
            stream << call[i][j];
        }
    }
    stream << direction;
    stream << remote;

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

    stream >> direction;
    stream >> remote;

    return (stream.status() == QDataStream::Ok);
}

QDataStream &operator<<(QDataStream & stream, const internal_state &state)
{
    for (int i = 0; i < N_FLOORS; i++)
        stream << state.call[i];

    return stream;
}

QDataStream &operator>>(QDataStream & stream, internal_state &state)
{
    for (int i = 0; i < N_FLOORS; i++)
        stream >> state.call[i];

    return stream;
}

//QDataStream &operator<<(QDataStream & stream, const elev_button_type_t &type)
//{
//    stream << (quint8) type;

//    return stream;
//}

//QDataStream &operator>>(QDataStream & stream, elev_button_type_t &type)
//{
//    quint8 tmp;
//    stream >> tmp;
//    type = (elev_button_type_t) tmp;

//    return stream;
//}
