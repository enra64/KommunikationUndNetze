#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>

#include "highnetwork.h"
#include "lownetwork.h"
#include "basenetwork.h"


namespace Ui {
class ChatWindow;
}

class ChatWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ChatWindow(QWidget *parent = 0);
    ~ChatWindow();
    void print(QString output);
    void error(QString errorDescription);
    void notify(QString msg);
    void connectionStatus(bool connectionOk);
    void setSendingUiEnabled(bool enable);
    void setConnectionUiEnabled(bool enable);

public slots:
    void receive();
    void onClientConnected(bool success);
private:
    Ui::ChatWindow *ui;
    BaseNetwork* mNetwork;
    void closeNetworkWithUi();
    void closeEvent(QCloseEvent *bar);
    bool mIsLowNetwork;
    QTimer* mTimer;
private slots:
    void on_sendButton_clicked();
    void on_clientConnectButton_clicked();
    void on_serverConnectButton_clicked();
    void on_sendText_returnPressed();
    void on_sendText_textChanged(const QString &arg1);
    void on_disconnectButton_clicked();
};

#endif // CHATWINDOW_H
