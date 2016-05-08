#ifndef MESSAGE_H
#define MESSAGE_H

#include <QString>

class Message
{
public:
    Message(QString msg, QString sendr);
    QString message, sender;
    int length();
    bool isEmpty();
};

#endif // MESSAGE_H
