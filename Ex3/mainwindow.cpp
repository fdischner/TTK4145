#include "mainwindow.h"
#include "networkmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QRadioButton>
#include <QTextEdit>
#include <QGroupBox>
#include <QStatusBar>
#include <QByteArray>
#include <QAbstractSocket>
#include <QtDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QLabel *label;
    QHBoxLayout *hbox;
    QVBoxLayout *vbox;

    hbox = new QHBoxLayout();
    label = new QLabel("Server IP:");
    hbox->addWidget(label);
    server_ip = new QLineEdit();
    server_ip->setText("129.241.187.161");
    hbox->addWidget(server_ip);
    label = new QLabel("Port:");
    hbox->addWidget(label);
    port_spinner = new QSpinBox();
    // FIXME: get this from some network define
    port_spinner->setMaximum(65535);
    port_spinner->setValue(33546);
    hbox->addWidget(port_spinner);
    vbox = new QVBoxLayout();
    tcp_button = new QRadioButton("TCP");
    vbox->addWidget(tcp_button);
    udp_button = new QRadioButton("UDP");
    vbox->addWidget(udp_button);
    hbox->addLayout(vbox);

    QWidget *w = new QWidget();
    vbox = new QVBoxLayout();
    w->setLayout(vbox);
    setCentralWidget(w);
    vbox->addLayout(hbox);

    hbox = new QHBoxLayout();
    vbox->addLayout(hbox);
    connect_button = new QPushButton("Connect");
    hbox->addWidget(connect_button);
    connect(connect_button, SIGNAL(clicked()), this, SLOT(onConnect()));
    disconnect_button = new QPushButton("Disconnect");
    disconnect_button->setEnabled(false);
    connect(disconnect_button, SIGNAL(clicked()), this, SLOT(onDisconnect()));
    hbox->addWidget(disconnect_button);
    simon_says_button = new QPushButton("Simon Says");
    connect(simon_says_button, SIGNAL(clicked()), this, SLOT(onSimonSays()));
    simon_says_button->setEnabled(false);
    hbox->addWidget(simon_says_button);

    label = new QLabel("Client Message:");
    vbox->addWidget(label);
    message_edit = new QTextEdit();
    vbox->addWidget(message_edit);
    hbox = new QHBoxLayout();
    vbox->addLayout(hbox);
    send_button = new QPushButton("Send");
    connect(send_button, SIGNAL(clicked()), this, SLOT(onSend()));
    hbox->addWidget(send_button);
    hbox->addStretch(1);
    QPushButton *clear = new QPushButton("Clear");
    hbox->addWidget(clear);
    connect(clear, SIGNAL(clicked()), message_edit, SLOT(clear()));

    label = new QLabel("Server Response:");
    vbox->addWidget(label);
    response_edit = new QTextEdit();
    response_edit->setReadOnly(true);
    vbox->addWidget(response_edit);

    tcp_button->setChecked(true);
    connect(tcp_button, SIGNAL(toggled(bool)), this, SLOT(onDisconnect()));

    statusBar()->showMessage("Disconnected");
    
    network_manager = new NetworkManager(this);
    connect(network_manager, SIGNAL(messageReceived(QByteArray)), this, SLOT(onMessageReceived(QByteArray)));
}

MainWindow::~MainWindow()
{

}

void MainWindow::onDisconnect()
{
    if (network_manager->socket == 0)
        return;

    send_button->setEnabled(false);
    simon_says_button->setEnabled(false);

    statusBar()->showMessage("Disconnecting...");
    network_manager->socket->disconnectFromHost();
}

void MainWindow::onConnect() {
    statusBar()->showMessage("Connecting...");
    connect_button->setEnabled(false);
    disconnect_button->setEnabled(true);
    network_manager->initSocket(tcp_button->isChecked() ? QAbstractSocket::TcpSocket : QAbstractSocket::UdpSocket, server_ip->text(), port_spinner->value());
    connect(network_manager->socket, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(network_manager->socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    // udp sockets don't emit the connected signal
    if (udp_button->isChecked())
        onConnected();
}

void MainWindow::onSend()
{
    network_manager->sendMessage(message_edit->toPlainText().toUtf8());
}

void MainWindow::onMessageReceived(const QByteArray& message)
{
    response_edit->append(message);
}

void MainWindow::onConnected()
{
    disconnect_button->setEnabled(true);
    send_button->setEnabled(true);
    statusBar()->showMessage("Connected");
    if (network_manager->socket->socketType() == QAbstractSocket::TcpSocket)
        simon_says_button->setEnabled(true);
}

void MainWindow::onDisconnected()
{
    disconnect_button->setEnabled(false);
    connect_button->setEnabled(true);
    statusBar()->showMessage("Disconnected");
}

void MainWindow::onSimonSays()
{
    simon_says_button->setEnabled(false);
    send_button->setEnabled(false);
    connect(network_manager, SIGNAL(messageReceived(QByteArray)), this, SLOT(onMessageReceivedSimon(QByteArray)));
    network_manager->sendMessage(QByteArray("Play Simon says"));
}

void MainWindow::onMessageReceivedSimon(const QByteArray &message)
{
    if (message.startsWith("Simon says "))
    {
        // respond with content, skipping 'Simon says '
        network_manager->sendMessage(message.mid(11));
    }
    else if (message == "Game over." || message == "Congratulations! Simon gave up.")
    {
        disconnect(network_manager, SIGNAL(messageReceived(QByteArray)), this, SLOT(onMessageReceivedSimon(QByteArray)));
        simon_says_button->setEnabled(true);
        send_button->setEnabled(true);
    }
}
