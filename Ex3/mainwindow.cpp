#include "mainwindow.h"
//#include <QApplication>
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


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QLabel *label;
    QHBoxLayout *hbox;
    QVBoxLayout *vbox;

    hbox = new QHBoxLayout();
    label = new QLabel("Server IP:");
    hbox->addWidget(label);
    QLineEdit *edit = new QLineEdit();
    edit->setText("129.241.187.0");
    hbox->addWidget(edit);
    label = new QLabel("Port:");
    hbox->addWidget(label);
    port_spinner = new QSpinBox();
    // FIXME: get this from some network define
    port_spinner->setMaximum(65535);
    port_spinner->setValue(34933);
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
    disconnect_button = new QPushButton("Disconnect");
    hbox->addWidget(disconnect_button);
    simon_says_button = new QPushButton("Simon Says");
    hbox->addWidget(simon_says_button);

    label = new QLabel("Client Message:");
    vbox->addWidget(label);
    message_edit = new QTextEdit();
    vbox->addWidget(message_edit);
    hbox = new QHBoxLayout();
    vbox->addLayout(hbox);
    send_button = new QPushButton("Send");
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

    connect(tcp_button, SIGNAL(toggled(bool)), this, SLOT(onTcpChecked(bool)));
    tcp_button->setChecked(true);

    statusBar()->showMessage("Disconnected");
    //setUnifiedTitleAndToolBarOnMac(true);
}

MainWindow::~MainWindow()
{

}

void MainWindow::onTcpChecked(bool checked)
{
    connect_button->setEnabled(checked);
    send_button->setEnabled(!checked);
    disconnect_button->setEnabled(false);
    simon_says_button->setEnabled(false);

    statusBar()->showMessage("Disconnected");
    // TODO: close socket
}
