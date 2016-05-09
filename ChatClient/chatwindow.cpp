#include "chatwindow.h"
#include "ui_chatwindow.h"

ChatWindow::ChatWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ChatWindow)
{
    ui->setupUi(this);

    mNetwork = new LowNetwork(this);

    QObject::connect(mNetwork, SIGNAL(messageReceived(Message)), this, SLOT(onMessageReceived(Message)));
    QObject::connect(mNetwork, SIGNAL(clientConnected(bool)), this, SLOT(onClientConnected(bool)));
    QObject::connect(mNetwork, SIGNAL(disconnect(QString, int)), this, SLOT(onDisconnect(QString, int)));
    QObject::connect(mNetwork, SIGNAL(closed(int)), this, SLOT(onNetworkClosed(int)));

    setSendingUiEnabled(false);
    setConnectionUiEnabled(true);
}

ChatWindow::~ChatWindow()
{
    closeNetworkWithUi();
    delete mNetwork;
    delete ui;
}

void ChatWindow::print(QString output){
    ui->chatHistory->append(output);
}

void ChatWindow::notify(QString output){
    print(output);
}

void ChatWindow::error(QString output){
    print(output);
}

void ChatWindow::onClientConnected(bool success){
    if(success)
        notify("Client connected successfully");
    else
        error("Could not accept a connection");
}

void ChatWindow::onDisconnect(QString name, int remainingConnections)
{
    notify(name + " disconnected!");
    if(remainingConnections == 0 && mNetwork->getConnectionState() == ConnectionState::SERVER)
        notify("Server still running, no clients remaining!");
}

void ChatWindow::onMessageReceived(Message msg)
{
    print(msg.getSender().getName() + ": " + msg.getMessage());
}

void ChatWindow::onNetworkClosed(int status)
{
    connectionStatus(false);
    if(status < 0)
        error("Could not close network");
    else
        notify("Network closed successfully");
}

void ChatWindow::connectionStatus(bool connectionOk){
    setSendingUiEnabled(connectionOk);
    setConnectionUiEnabled(!connectionOk);
}

void ChatWindow::setSendingUiEnabled(bool enable){
    const bool hasText = !ui->sendText->text().isEmpty();
    ui->sendButton->setEnabled(enable && hasText);
    ui->sendText->setEnabled(enable);
}

void ChatWindow::setConnectionUiEnabled(bool enable){
    ui->clientConnectButton->setEnabled(enable);
    ui->serverConnectButton->setEnabled(enable);
    ui->hostText->setEnabled(enable);
    ui->portText->setEnabled(enable);
    ui->disconnectButton->setEnabled(!enable);
}

void ChatWindow::closeNetworkWithUi()
{
    // emits a signal, dont need to catch errors
    mNetwork->closeNetwork();
}

void ChatWindow::on_sendButton_clicked()
{
    const QString sendText = ui->sendText->text();
    if(mNetwork->send(sendText) < 0)
        error("'" + sendText + "' could not be sent");
    else
        print("You: " + sendText);
    ui->sendText->clear();
}

void ChatWindow::on_clientConnectButton_clicked()
{
    int status;
    switch(status = mNetwork->client(ui->hostText->text(), ui->portText->text())){
        default:
            notify("Connected to server");
            break;
        case -1:
            error("Port no integer");
            break;
        case -2:
            error("Host could not be resolved");
            break;
        case -3:
            error("Could not socket()");
            break;
        case -4:
            error("could not connect()");
        break;
    }
    connectionStatus(status >= 0);
}

void ChatWindow::closeEvent(QCloseEvent *bar)
{
    closeNetworkWithUi();
    bar->accept();
}

void ChatWindow::on_serverConnectButton_clicked()
{
    int status;
    switch(status = mNetwork->server(ui->portText->text())){
        case -1:
            error("Port no integer!");
            break;
        case -2:
            error("Could not do socket()");
            break;
        case -3:
            error("could not bind()");
            break;
        case -4:
            error("could not listen()");
            break;
        case -5:
            error("could not accept() incoming connection");
            break;
        default:
            print("waiting for a connection");
    }
    connectionStatus(status >= 0);
}

void ChatWindow::on_sendText_returnPressed()
{
    on_sendButton_clicked();
}

void ChatWindow::on_sendText_textChanged(const QString &arg1)
{
    ui->sendButton->setEnabled(!arg1.isEmpty());
}

void ChatWindow::on_disconnectButton_clicked()
{
    closeNetworkWithUi();
}
