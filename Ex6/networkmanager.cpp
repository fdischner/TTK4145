#include "networkmanager.h"
#include <QTcpSocket>
#include <QUdpSocket>
#include <QNetworkInterface>
#include <QtDebug>

NetworkManager::NetworkManager(QObject *parent) :
    QObject(parent), socket(0)
{
}

void NetworkManager::initSocket(QAbstractSocket::SocketType type, const QString& address, quint16 port)
{
    if (socket != 0)
        delete socket;
    
    this->address = address;
    this->port = port;

    if (type == QAbstractSocket::TcpSocket) {
        socket = new QTcpSocket(this);
        socket->connectToHost(address, port);
    } else {
        QUdpSocket *udp = new QUdpSocket(this);
        udp->bind(QHostAddress(address), port, QUdpSocket::ShareAddress|QUdpSocket::ReuseAddressHint);
        socket = udp;
    }

    connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
}

void NetworkManager::sendMessage(const QByteArray& message)
{
    QByteArray newMessage = message;
    newMessage.append('\0');

    if (socket->socketType() == QAbstractSocket::TcpSocket)
    {
        socket->write(newMessage);
    }
    else if (socket->socketType() == QAbstractSocket::UdpSocket)
    {
        QUdpSocket *udp = static_cast<QUdpSocket *>(socket);
        udp->writeDatagram(newMessage, QHostAddress(address), port);
    }
}

void NetworkManager::onReadyRead()
{
    if (socket->socketType() == QAbstractSocket::UdpSocket)
    {
        QUdpSocket *udp = static_cast<QUdpSocket *>(socket);

        while (udp->hasPendingDatagrams())
        {
            QHostAddress sender;

            data.resize(udp->pendingDatagramSize());

            if (udp->readDatagram(data.data(), data.size(), &sender, 0) == -1)
                return;

            // don't emit message if it came from us
            /*foreach (const QHostAddress &address, QNetworkInterface::allAddresses())
            {
                if (address == sender)
                {
                    data.clear();
                    break;
                }
            }*/

            if (data.endsWith('\0'))
                data.chop(1);

            if (!data.isEmpty())
                emit messageReceived(data);
        }
    }
    else
    {
        while (socket->bytesAvailable() > 0)
        {
            QByteArray tmp = socket->readAll();
            int from, pos;

            if (tmp.isEmpty())
                return;

            data.append(tmp);

            from = 0;
            while (true)
            {
                pos = data.indexOf('\0', from);
                if (pos == -1)
                    break;
                if (pos != from)
                    emit messageReceived(data.mid(from, pos - from));
                from = pos + 1;
            }

            if (from > 0)
                data = data.mid(from);
        }
    }
}
