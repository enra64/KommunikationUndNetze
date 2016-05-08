#include "message.h"

Message::Message(QString msg, QString sendr) : message(msg), sender(sendr)
{
}

int Message::length()
{
    return message.length();
}

bool Message::isEmpty()
{
    return message.isEmpty();
}
