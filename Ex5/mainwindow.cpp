#include "mainwindow.h"
//#include "networkmanager.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QtDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    //should be on h file?
    QHBoxLayout *hbox;
    QVBoxLayout *vbox, *vbox_aux;
    QLabel *label;
    QLabel *floor1, *floor2, *floor3, *floor4;
    QPushButton *elev_f1_up, *elev_f1_down, *elev_f1_in;
    QPushButton *elev_f2_up, *elev_f2_down, *elev_f2_in;
    QPushButton *elev_f3_up, *elev_f3_down, *elev_f3_in;
    QPushButton *elev_f4_up, *elev_f4_down, *elev_f4_in;

    vbox = new QVBoxLayout();

    label = new QLabel("Elevator simulator");
    vbox->addWidget(label);

    hbox = new QHBoxLayout();
    label = new QLabel("Inside buttons");
    vbox->addWidget(label);
    elev_f1_in = new QPushButton("elev_f1_in");
    //connect(elev_f1_in, SIGNAL(clicked()), this, SLOT(onConnect()));
    hbox->addWidget(elev_f1_in);
    elev_f2_in = new QPushButton("elev_f2_in");
    //connect(elev_f2_in, SIGNAL(clicked()), this, SLOT(onConnect()));
    hbox->addWidget(elev_f2_in);
    elev_f3_in = new QPushButton("elev_f3_in");
    //connect(elev_f3_in, SIGNAL(clicked()), this, SLOT(onConnect()));
    hbox->addWidget(elev_f3_in);
    elev_f4_in = new QPushButton("elev_f4_in");
    //connect(elev_f4_in, SIGNAL(clicked()), this, SLOT(onConnect()));
    hbox->addWidget(elev_f4_in);
    vbox->addLayout(hbox);

    hbox = new QHBoxLayout();
    vbox_aux = new QVBoxLayout();
    floor4 = new QLabel("4th floor off");
    hbox->addWidget(floor4);
    elev_f4_up = new QPushButton("elev_f4_up");
    //connect(elev_f4_up, SIGNAL(clicked()), this, SLOT(onConnect()));
    vbox_aux->addWidget(elev_f4_up);
    elev_f4_down = new QPushButton("elev_f4_down");
    //connect(elev_f4_down, SIGNAL(clicked()), this, SLOT(onConnect()));
    vbox_aux->addWidget(elev_f4_down);
    hbox->addLayout(vbox_aux);
    vbox->addLayout(hbox);

    hbox = new QHBoxLayout();
    vbox_aux = new QVBoxLayout();
    floor3 = new QLabel("3rd floor off");
    hbox->addWidget(floor3);
    elev_f3_up = new QPushButton("elev_f3_up");
    //connect(elev_f3_up, SIGNAL(clicked()), this, SLOT(onConnect()));
    vbox_aux->addWidget(elev_f3_up);
    elev_f3_down = new QPushButton("elev_f3_down");
    //connect(elev_f3_down, SIGNAL(clicked()), this, SLOT(onConnect()));
    vbox_aux->addWidget(elev_f3_down);
    hbox->addLayout(vbox_aux);
    vbox->addLayout(hbox);

    hbox = new QHBoxLayout();
    vbox_aux = new QVBoxLayout();
    floor2 = new QLabel("2nd floor off");
    hbox->addWidget(floor2);
    elev_f2_up = new QPushButton("elev_f2_up");
    //connect(elev_f2_up, SIGNAL(clicked()), this, SLOT(onConnect()));
    vbox_aux->addWidget(elev_f2_up);
    elev_f2_down = new QPushButton("elev_f2_down");
    //connect(elev_f2_down, SIGNAL(clicked()), this, SLOT(onConnect()));
    vbox_aux->addWidget(elev_f2_down);
    hbox->addLayout(vbox_aux);
    vbox->addLayout(hbox);

    hbox = new QHBoxLayout();
    vbox_aux = new QVBoxLayout();
    floor1 = new QLabel("1st floor off");
    hbox->addWidget(floor1);
    elev_f1_up = new QPushButton("elev_f1_up");
    //connect(elev_f1_up, SIGNAL(clicked()), this, SLOT(onConnect()));
    vbox_aux->addWidget(elev_f1_up);
    elev_f1_down = new QPushButton("elev_f1_down");
    //connect(elev_f1_down, SIGNAL(clicked()), this, SLOT(onConnect()));
    vbox_aux->addWidget(elev_f1_down);
    hbox->addLayout(vbox_aux);
    vbox->addLayout(hbox);

    //Add graphic elements to layout
    QWidget *w = new QWidget();
    w->setLayout(vbox);
    setCentralWidget(w);
}

MainWindow::~MainWindow()
{
}
