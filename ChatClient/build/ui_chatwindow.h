/********************************************************************************
** Form generated from reading UI file 'chatwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHATWINDOW_H
#define UI_CHATWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ChatWindow
{
public:
    QWidget *centralWidget;
    QGridLayout *gridLayout;
    QVBoxLayout *verticalLayout;
    QTextBrowser *chatHistory;
    QHBoxLayout *horizontalLayout;
    QLineEdit *sendText;
    QPushButton *sendButton;
    QHBoxLayout *horizontalLayout_2;
    QLineEdit *hostText;
    QLineEdit *portText;
    QPushButton *clientConnectButton;
    QPushButton *serverConnectButton;
    QPushButton *disconnectButton;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *ChatWindow)
    {
        if (ChatWindow->objectName().isEmpty())
            ChatWindow->setObjectName(QStringLiteral("ChatWindow"));
        ChatWindow->resize(400, 300);
        centralWidget = new QWidget(ChatWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        gridLayout = new QGridLayout(centralWidget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        chatHistory = new QTextBrowser(centralWidget);
        chatHistory->setObjectName(QStringLiteral("chatHistory"));

        verticalLayout->addWidget(chatHistory);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        sendText = new QLineEdit(centralWidget);
        sendText->setObjectName(QStringLiteral("sendText"));

        horizontalLayout->addWidget(sendText);

        sendButton = new QPushButton(centralWidget);
        sendButton->setObjectName(QStringLiteral("sendButton"));

        horizontalLayout->addWidget(sendButton);


        verticalLayout->addLayout(horizontalLayout);


        gridLayout->addLayout(verticalLayout, 0, 0, 1, 1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        hostText = new QLineEdit(centralWidget);
        hostText->setObjectName(QStringLiteral("hostText"));

        horizontalLayout_2->addWidget(hostText);

        portText = new QLineEdit(centralWidget);
        portText->setObjectName(QStringLiteral("portText"));

        horizontalLayout_2->addWidget(portText);

        clientConnectButton = new QPushButton(centralWidget);
        clientConnectButton->setObjectName(QStringLiteral("clientConnectButton"));

        horizontalLayout_2->addWidget(clientConnectButton);

        serverConnectButton = new QPushButton(centralWidget);
        serverConnectButton->setObjectName(QStringLiteral("serverConnectButton"));

        horizontalLayout_2->addWidget(serverConnectButton);

        disconnectButton = new QPushButton(centralWidget);
        disconnectButton->setObjectName(QStringLiteral("disconnectButton"));

        horizontalLayout_2->addWidget(disconnectButton);


        gridLayout->addLayout(horizontalLayout_2, 2, 0, 1, 1);

        ChatWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(ChatWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 400, 25));
        ChatWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(ChatWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        ChatWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(ChatWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        ChatWindow->setStatusBar(statusBar);

        retranslateUi(ChatWindow);

        QMetaObject::connectSlotsByName(ChatWindow);
    } // setupUi

    void retranslateUi(QMainWindow *ChatWindow)
    {
        ChatWindow->setWindowTitle(QApplication::translate("ChatWindow", "ChatWindow", 0));
        sendText->setPlaceholderText(QApplication::translate("ChatWindow", "message", 0));
        sendButton->setText(QApplication::translate("ChatWindow", "Send", 0));
        hostText->setText(QApplication::translate("ChatWindow", "localhost", 0));
        hostText->setPlaceholderText(QApplication::translate("ChatWindow", "host", 0));
        portText->setText(QApplication::translate("ChatWindow", "2048", 0));
        portText->setPlaceholderText(QApplication::translate("ChatWindow", "port", 0));
        clientConnectButton->setText(QApplication::translate("ChatWindow", "Connect as client", 0));
        serverConnectButton->setText(QApplication::translate("ChatWindow", "Start Server", 0));
        disconnectButton->setText(QApplication::translate("ChatWindow", "Disconnect", 0));
    } // retranslateUi

};

namespace Ui {
    class ChatWindow: public Ui_ChatWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHATWINDOW_H
