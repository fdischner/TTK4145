#ifndef CONTROL_H
#define CONTROL_H

#include <QObject>

class Elevator;

class Control : public QObject
{
    Q_OBJECT
public:
    explicit Control(QObject *parent = 0);

signals:

public slots:

private:
    Elevator *elevator;
};

#endif // CONTROL_H
