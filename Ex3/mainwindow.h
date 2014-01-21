#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QPushButton;
class QRadioButton;
class QLineEdit;
class QSpinBox;
class QTextEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void onTcpChecked(bool checked);

private:
    QRadioButton *tcp_button;
    QRadioButton *udp_button;
    QPushButton *connect_button;
    QPushButton *disconnect_button;
    QPushButton *simon_says_button;
    QPushButton *send_button;
    QLineEdit *server_ip;
    QSpinBox *port_spinner;
    QTextEdit *message_edit;
    QTextEdit *response_edit;
};

#endif // MAINWINDOW_H
