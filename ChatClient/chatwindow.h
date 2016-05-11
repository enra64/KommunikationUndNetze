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
public slots:
    void onClientConnected(NetworkError success);
    void onDisconnect(QString name, int remainingConnections);
    void onMessageReceived(Message msg);
    void onNetworkClosed(int status);
private:
    void print(QString output);
    void error(QString errorDescription);
    void notify(QString msg);
    void connectionStatus(bool connectionOk);
    void setSendingUiEnabled(bool enable);
    void setConnectionUiEnabled(bool enable);
    Ui::ChatWindow *ui;
    BaseNetwork* mNetwork = nullptr;
    void closeNetworkWithUi();
    void closeEvent(QCloseEvent *bar);
    void loadNetwork(bool kn);
    QTimer* mTimer;
private slots:
    void on_sendButton_clicked();
    void on_clientConnectButton_clicked();
    void on_serverConnectButton_clicked();
    void on_sendText_returnPressed();
    void on_sendText_textChanged(const QString &arg1);
    void on_disconnectButton_clicked();
    void on_knApiRadio_toggled(bool checked);
    void on_unixApiRadio_toggled(bool checked);
};

#endif // CHATWINDOW_H
