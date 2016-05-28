#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>

#include "network.h"


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
    /// Called whenever a data message has been received
    void onReceive(const DataMessage& d);

    /// Called whenever a peer has been added or removed
    void onPeerListUpdate(std::vector<Peer> peerList, std::vector<Peer>, int sizeDiffSize);

    /// Called when the network has been closed
    void onNetworkClosed(int status);

private:
    /// standard output method
    void print(QString output);

    /// warn of error in chat window
    void error(QString errorDescription);

    /// notify user in chat window
    void notify(QString msg);


    void connectionStatus(bool connectionOk);


    void setSendingUiEnabled(bool enable);

    /// set connection ui (connect, open server, port, host) state
    void setConnectionUiEnabled(bool enable);

    /// get the currently selected peer
    const Peer& getActivePeer();

    /// reference to our ui object
    Ui::ChatWindow *ui;

    /// currently used network system
    Network* mNetwork = nullptr;

    /// no idea
    void closeNetworkWithUi();

    /// catch close event to close network correctly
    void closeEvent(QCloseEvent *bar);

    /// either load the server or the client system
    NetworkError& loadNetwork(bool isServer);

    QTimer* mTimer;

    std::vector<Peer> mPeerList;

private slots:
    void on_sendButton_clicked();
    void on_clientConnectButton_clicked();
    void on_serverConnectButton_clicked();
    void on_sendText_returnPressed();
    void on_sendText_textChanged(const QString &arg1);
    void on_disconnectButton_clicked();
};

#endif // CHATWINDOW_H
