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
    
    this->address = QHostAddress(address);
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

    // TCP sockets are just a stream so we need a way to delimit messages
    // UDP encodes the message size in the datagram so this is not necessary
    // Note: this means TCP messages cannot contain null bytes, but UDP messages can
    if (socket->socketType() == QAbstractSocket::TcpSocket)
        newMessage.append('\0');

    if (socket->socketType() == QAbstractSocket::TcpSocket)
    {
        socket->write(newMessage);
    }
    else if (socket->socketType() == QAbstractSocket::UdpSocket)
    {
        QUdpSocket *udp = static_cast<QUdpSocket *>(socket);
        udp->writeDatagram(newMessage, address, port);
    }
}

void NetworkManager::onReadyRead()
{
    if (socket->socketType() == QAbstractSocket::UdpSocket)
    {
        QUdpSocket *udp = static_cast<QUdpSocket *>(socket);

        // process all pending messages
        while (udp->hasPendingDatagrams())
        {
            QHostAddress sender;

            // ensure buffer is large enough for the message
            data.resize(udp->pendingDatagramSize());

            // read the message and the sender
            if (udp->readDatagram(data.data(), data.size(), &sender, 0) == -1)
                return;

            // don't emit message if it came from us and peer address is not localhost
            if (address != QHostAddress::LocalHost)
            {
                foreach (const QHostAddress &address, QNetworkInterface::allAddresses())
                {
                    if (address == sender)
                    {
                        data.clear();
                        break;
                    }
                }
            }

            // don't emit empty messages
            if (!data.isEmpty())
                emit messageReceived(data);
        }
    }
    else
    {
        while (socket->bytesAvailable() > 0)
        {
            // read all bytes available on the socket
            QByteArray tmp = socket->readAll();
            int from, pos;

            // if we didn't read anything, then there is nothing to do
            if (tmp.isEmpty())
                return;

            // append read bytes to previous buffer
            data.append(tmp);

            // process all complete messages in the buffer
            from = 0;
            while (true)
            {
                // find the end of the current message
                pos = data.indexOf('\0', from);

                // if no delimiter is found, then we haven't received a complete message
                if (pos == -1)
                    break;

                // only emit message if size is greater than zero
                if (pos != from)
                    emit messageReceived(data.mid(from, pos - from));

                // move to start of next message
                from = pos + 1;
            }

            // remove any messages that were already processed
            if (from > 0)
                data = data.mid(from);
        }
    }
}
