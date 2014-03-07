#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QAbstractSocket>
#include <QHostAddress>

class NetworkManager : public QObject
{
    Q_OBJECT
    
public:
    explicit NetworkManager(QObject *parent = 0);
    void initSocket(QAbstractSocket::SocketType type, const QString& address, quint16 port);
    
signals:
    void messageReceived(const QByteArray& message);

public slots:
    void sendMessage(const QByteArray& message);

private slots:
    void onReadyRead();

public:
    QAbstractSocket *socket;

private:
    QByteArray data;
    QHostAddress address;
    quint16 port;
};

#endif // NETWORKMANAGER_H
