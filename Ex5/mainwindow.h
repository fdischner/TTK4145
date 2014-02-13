#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

//class NetworkManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
/*  void onConnect();
    void onDisconnect();
    void onSend();
    void onMessageReceived(const QByteArray& message);
    void onConnected();
    void onDisconnected();
*/
private:
    //NetworkManager *network_manager;
};

#endif // MAINWINDOW_H
