#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QByteArray>
#include <QString>

class QAbstractSocket;

class NetworkManager : public QObject
{
    Q_OBJECT
    
public:
    enum SocketType {
      TCP,
      UDP
    };
    
    explicit NetworkManager(QObject *parent = 0);
    void initSocket(SocketType type, const QString& address, quint16 port);
    void sendMessage(const QByteArray& message);
    QByteArray readMessage(void);
    
signals:
    void messageReady();

public slots:

private slots:
    void onReadyRead();

private:
    QAbstractSocket *socket;
    SocketType type;
    QString address;
    quint16 port;
};

#endif // NETWORKMANAGER_H
