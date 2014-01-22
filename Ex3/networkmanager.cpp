#include "networkmanager.h"
#include <QTcpSocket>
#include <QUdpSocket>
#include <QtDebug>

NetworkManager::NetworkManager(QObject *parent) :
    QObject(parent), socket(0)
{
}

void NetworkManager::initSocket(SocketType type, 
				const QString& address, quint16 port) {
    if (socket != 0)
      delete socket;
    
    this->address = address;
    this->port = port;
    this->type = type;

    if (type == TCP) {
      socket = new QTcpSocket(this);
      socket->connectToHost(address, port);
    } else {
      QUdpSocket *udp = new QUdpSocket(this);
      udp->bind(QHostAddress(address), port);
      socket = udp;
    }
    connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
}

void NetworkManager::sendMessage(const QByteArray& message) {
    if (type == TCP) {
	socket->write(message);
    }
    else
    {
	QUdpSocket *udp = static_cast<QUdpSocket *>(socket);
	udp->writeDatagram(message, QHostAddress(address), port);
    }
}

QByteArray NetworkManager::readMessage(void) {
    if (type == TCP) {
	return socket->read(1024);
    } else {
	QUdpSocket *udp = static_cast<QUdpSocket *>(socket);
	QByteArray datagram;
	     if (udp->hasPendingDatagrams()) {
	       datagram.resize(udp->pendingDatagramSize());
	       //QHostAddress sender;
	       udp->readDatagram(datagram.data(), datagram.size(), 0, 0);
	       if (!datagram.startsWith("You said:"))	//CHANGE FOR THE HOST IP
		 datagram.clear();
	     }
	return datagram;
    }
}

void NetworkManager::onReadyRead()
{
    emit messageReady();
}