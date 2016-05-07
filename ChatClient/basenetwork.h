#ifndef BASENETWORK_H
#define BASENETWORK_H

#include <QObject>

class BaseNetwork : public QObject
{
    Q_OBJECT
public:
    explicit BaseNetwork(QObject *parent = 0);

signals:

public slots:

private:
};

#endif // BASENETWORK_H
