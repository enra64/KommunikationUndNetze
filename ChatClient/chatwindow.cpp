#include "chatwindow.h"
#include "ui_chatwindow.h"

ChatWindow::ChatWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ChatWindow)
{
    ui->setupUi(this);

    // CHANGE BOTH
    mIsLowNetwork = false;
    mNetwork = new LowNetwork(this);
    QObject::connect(mNetwork, SIGNAL(clientConnected(bool)), this, SLOT(onClientConnected(bool)));

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

void ChatWindow::connectionStatus(bool connectionOk){
    setSendingUiEnabled(connectionOk);
    setConnectionUiEnabled(!connectionOk);
    if(connectionOk){
        mTimer = new QTimer(this);
        QObject::connect(
                    mTimer,
                    SIGNAL(timeout()),
                    this,
                    SLOT(receive()));
        mTimer->start(4);
    }
    else if(mTimer != nullptr){
        QObject::disconnect(mTimer, SIGNAL(timeout()), this, SLOT(receive()));
        delete mTimer;
        mTimer = nullptr;
    }
}

void ChatWindow::receive(){
    std::vector<Message> mResults;
    mNetwork->receive(mResults);
    for(Message m : mResults)
        print(m.sender + m.message);
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
    connectionStatus(false);
    if(mNetwork->closeNetwork() >= 0)
        print("Closed network successfully");
    else
        print("Could not close network");
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
