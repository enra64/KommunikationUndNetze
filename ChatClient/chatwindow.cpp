#include "chatwindow.h"
#include "ui_chatwindow.h"

ChatWindow::ChatWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ChatWindow)
{
    ui->setupUi(this);

    setSendingUiEnabled(false);
    setConnectionUiEnabled(false);
    ui->disconnectButton->setEnabled(false);
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

void ChatWindow::onClientConnected(NetworkError success){
    if(success == NetworkError::ERROR_NO_ERROR)
        notify("Client connected successfully");
    else
        error("Could not accept a connection");
}

void ChatWindow::onDisconnect(QString name, int remainingConnections)
{
    notify(name + " disconnected!");
    if(remainingConnections == 0 && mNetwork->getConnectionState() == ConnectionState::SERVER)
        notify("Server still running, 0 clients.");
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
    //ui->disconnectButton->setEnabled(connectionOk);
}

void ChatWindow::setSendingUiEnabled(bool enable){
    const bool hasText = !ui->sendText->text().isEmpty();
    ui->sendButton->setEnabled(enable && hasText);
    ui->sendText->setEnabled(enable);
}

void ChatWindow::setConnectionUiEnabled(bool enable){
    ui->clientConnectButton->setEnabled(enable);
    ui->serverConnectButton->setEnabled(enable);
    ui->nameEdit->setEnabled(enable);
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
    NetworkError status = mNetwork->client(ui->hostText->text(), ui->portText->text());
    switch(status){
        case NetworkError::MAKE_CONTACT_FAILED:
            error("make_contact() failed");
            break;
        case NetworkError::HOST_NOT_RESOLVED:
            error("Host could not be resolved");
            break;
        case NetworkError::PORT_NO_INTEGER:
            error("Port no integer");
            break;
        case NetworkError::SOCKET_FAILED:
            error("socket() failed");
            break;
        case NetworkError::CONTACT_FAILED:
            error("connect() failed");
            break;
        case NetworkError::BIND_FAILED:
            error("bind() failed");
            break;
    case NetworkError::ACCEPT_FAILED:
    case NetworkError::AWAIT_CONTACT_FAILED:
    case NetworkError::LISTEN_FAILED:
        error("got a server error when using client()?");
        break;
    case NetworkError::ERROR_NO_ERROR:
        notify("Connected to server!");
        break;
    default:
        error("Unknown error occured");
    }
    connectionStatus(status == NetworkError::ERROR_NO_ERROR);
}

void ChatWindow::closeEvent(QCloseEvent *bar)
{
    closeNetworkWithUi();
    bar->accept();
}

void ChatWindow::on_serverConnectButton_clicked()
{
    NetworkError status = mNetwork->server(ui->portText->text());
    switch(status){
        case NetworkError::AWAIT_CONTACT_FAILED:
            error("make_contact()");
            break;
        case NetworkError::LISTEN_FAILED:
            error("Could not listen");
            break;
        case NetworkError::PORT_NO_INTEGER:
            error("Port no integer");
            break;
        case NetworkError::SOCKET_FAILED:
            error("could not socket()");
            break;
        case NetworkError::BIND_FAILED:
            error("could not bind()");
            break;
        case NetworkError::ACCEPT_FAILED:
            error("could not accpt()");
            break;
        case NetworkError::MAKE_CONTACT_FAILED:
        case NetworkError::HOST_NOT_RESOLVED:
            error("got a client error when using server()?");
            break;
        case NetworkError::ERROR_NO_ERROR:
            notify("Server started!");
            break;
        default:
            error("Unknown error occured");
    }
    connectionStatus(status == NetworkError::ERROR_NO_ERROR);
}

void ChatWindow::on_sendText_returnPressed()
{
    if(ui->sendButton->isEnabled())
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

void ChatWindow::loadNetwork(bool kn)
{
    // kill old networking solution
    if(mNetwork != nullptr){
        mNetwork->closeNetwork();
        delete mNetwork;
    }
    // create new network system
    if(kn)
        mNetwork = new HighNetwork(this);
    else
        mNetwork = new LowNetwork(this);

    QObject::connect(mNetwork, SIGNAL(messageReceived(Message)), this, SLOT(onMessageReceived(Message)));
    QObject::connect(mNetwork, SIGNAL(clientConnected(NetworkError)), this, SLOT(onClientConnected(NetworkError)));
    QObject::connect(mNetwork, SIGNAL(disconnect(QString, int)), this, SLOT(onDisconnect(QString, int)));
    QObject::connect(mNetwork, SIGNAL(closed(int)), this, SLOT(onNetworkClosed(int)));

    setSendingUiEnabled(false);
    setConnectionUiEnabled(true);
}

void ChatWindow::on_knApiRadio_toggled(bool useKn)
{
    if(useKn)
        loadNetwork(useKn);
}

void ChatWindow::on_unixApiRadio_toggled(bool useUnix)
{
    if(useUnix)
        loadNetwork(!useUnix);
}
