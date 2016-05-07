#ifndef BASENETWORK_H
#define BASENETWORK_H

#include <QObject>

#include <QString>
#include <QtConcurrent/QtConcurrent>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

class BaseNetwork : public QObject
{
    Q_OBJECT
public:
    explicit BaseNetwork(QObject *parent = 0);
    int closeNetwork();
    size_t receive(std::vector<QString>& msg);
    int send(const QString msg);
    int server(const QString port);
    int client(const QString host, const QString port);
signals:

public slots:

protected:
    QString networkToString(int fd, size_t& receiveLength);
    bool parsePort(const QString port, short& shortPort);
    int mZeroLengthMsgCount = 0;
    char mBuffer[1024];
};

#endif // BASENETWORK_H
