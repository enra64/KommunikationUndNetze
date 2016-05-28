#include "chatwindow.h"
#include "ui_chatwindow.h"

ChatWindow::ChatWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ChatWindow) {
    ui->setupUi(this);

    setSendingUiEnabled(false);
    setConnectionUiEnabled(true);
    ui->disconnectButton->setEnabled(false);
}

ChatWindow::~ChatWindow() {
    closeNetworkWithUi();
    delete mNetwork;
    delete ui;
}

void ChatWindow::print(QString output) {
    ui->chatHistory->append(output);
}

void ChatWindow::notify(QString output) {
    ui->chatHistory->insertHtml(output.prepend("<span style=\"color:blue;\">").append("</span>"));
}

void ChatWindow::error(QString output) {
    ui->chatHistory->insertHtml(output.prepend("<span style=\"color:red;\">").append("</span>"));
}

void ChatWindow::onPeerListUpdate(std::vector<Peer> peerList, std::vector<Peer>, int sizeDiffSize) {
    ui->peerList->clear();
    for(const Peer& p : peerList)
        ui->peerList->addItem(p.getName());
    switch(sizeDiffSize) {
    case -1:
        notify("1 client disconnected");
        break;
    case 1:
        notify("1 client connected");
        break;
    default:
        if(sizeDiffSize < 0)
            notify(sizeDiffSize + " clients disconnected");
        else
            notify(sizeDiffSize + " clients connected");
    }
}
void ChatWindow::onReceive(const DataMessage& msg) {
    print(msg.getSource().getName() + ": " + msg.getMessage());
}

void ChatWindow::onNetworkClosed(int status) {
    connectionStatus(false);
    mPeerList.clear();
    if(status < 0)
        error("Could not close network");
    else
        notify("Network closed successfully");
}

void ChatWindow::connectionStatus(bool connectionOk) {
    setSendingUiEnabled(connectionOk);
    setConnectionUiEnabled(!connectionOk);
}

void ChatWindow::setSendingUiEnabled(bool enable) {
    const bool hasText = !ui->sendText->text().isEmpty();
    ui->sendButton->setEnabled(enable && hasText);
    ui->sendText->setEnabled(enable);
}

void ChatWindow::setConnectionUiEnabled(bool enable) {
    ui->clientConnectButton->setEnabled(enable);
    ui->serverConnectButton->setEnabled(enable);
    ui->nameEdit->setEnabled(enable);
    ui->hostText->setEnabled(enable);
    ui->portText->setEnabled(enable);
    ui->disconnectButton->setEnabled(!enable);
}

const Peer& ChatWindow::getActivePeer() {
    int activeRow = ui->peerList->currentIndex().row();
    return mPeerList.at(activeRow);
}

void ChatWindow::closeNetworkWithUi() {
    // emits a signal, dont need to catch errors
    mNetwork->closeNetwork();
}

void ChatWindow::on_sendButton_clicked() {
    const QString sendText = ui->sendText->text();
    if(mNetwork->send_(sendText, getActivePeer()) < 0)
        error("'" + sendText + "' could not be sent");
    else
        print("You: " + sendText);
    ui->sendText->clear();
}

void ChatWindow::on_clientConnectButton_clicked() {
    NetworkError status = loadNetwork(false);
    switch(status) {
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

void ChatWindow::closeEvent(QCloseEvent *bar) {
    closeNetworkWithUi();
    bar->accept();
}

void ChatWindow::on_serverConnectButton_clicked() {
    NetworkError status = loadNetwork(true);
    switch(status) {
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

void ChatWindow::on_sendText_returnPressed() {
    if(ui->sendButton->isEnabled())
        on_sendButton_clicked();
}

void ChatWindow::on_sendText_textChanged(const QString &arg1) {
    ui->sendButton->setEnabled(!arg1.isEmpty());
}

void ChatWindow::on_disconnectButton_clicked() {
    closeNetworkWithUi();
}

NetworkError& ChatWindow::loadNetwork(bool isServer) {
    // kill old networking solution
    if(mNetwork != nullptr) {
        mNetwork->closeNetwork();
        mPeerList.clear();
        delete mNetwork;
    }

    // instantiate networking class
    static NetworkError result = NetworkError::ERROR_NO_ERROR;
    if(isServer)
        mNetwork = new Server(ui->portText->text(), result, this);
    else
        mNetwork = new Client(ui->hostText->text(), ui->portText->text(), ui->nameEdit->text(), result, this);

    QObject::connect(mNetwork, SIGNAL(received(DataMessage)), this, SLOT(onReceive(DataMessage)));
    QObject::connect(mNetwork, SIGNAL(peerListUpdated(std::vector<Peer>,std::vector<Peer>,int)), this, SLOT(onPeerListUpdate(std::vector<Peer>,std::vector<Peer>,int)));
    QObject::connect(mNetwork, SIGNAL(networkClosed(int)), this, SLOT(onNetworkClosed(int)));

    setSendingUiEnabled(false);
    setConnectionUiEnabled(true);

    return result;
}
