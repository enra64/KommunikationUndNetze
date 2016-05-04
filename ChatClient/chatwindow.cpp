#include "chatwindow.h"
#include "ui_chatwindow.h"

ChatWindow::ChatWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ChatWindow)
{
    ui->setupUi(this);
    mNetwork = new Network(this);
    setSendingUiEnabled(false);
}

ChatWindow::~ChatWindow()
{
    mNetwork->closeNetwork();
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

void ChatWindow::connectionStatus(bool connectionOk){
    setSendingUiEnabled(connectionOk);
    mTimer = new QTimer(this);
    QObject::connect(
                mTimer,
                SIGNAL(timeout()),
                mNetwork,
                SLOT(pollingRead()));
    mTimer->start(4);
}

void ChatWindow::checkNetwork(){
    mNetwork->pollingRead();
}

void ChatWindow::setSendingUiEnabled(bool enable){
    ui->sendButton->setEnabled(enable);
    ui->sendText->setEnabled(enable);
}

void ChatWindow::setConnectionUiEnabled(bool enable){
    ui->clientConnectButton->setEnabled(enable);
    ui->serverConnectButton->setEnabled(enable);
    ui->hostText->setEnabled(enable);
    ui->portText->setEnabled(enable);
    ui->disconnectButton->setEnabled(!enable);
}

void ChatWindow::on_sendButton_clicked()
{
    mNetwork->send(ui->sendText->text());
    ui->chatHistory->append("You: " + ui->sendText->text());
    ui->sendText->clear();
}

void ChatWindow::on_clientConnectButton_clicked()
{
    mNetwork->client(ui->hostText->text(), ui->portText->text());
}

void ChatWindow::closeEvent(QCloseEvent *bar)
{
    mNetwork->closeNetwork();
    bar->accept();
}

void ChatWindow::on_serverConnectButton_clicked()
{
    mNetwork->server(ui->portText->text());
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
    mNetwork->closeNetwork();
    setSendingUiEnabled(false);
    setConnectionUiEnabled(true);
}
