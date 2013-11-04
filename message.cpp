#include "message.h"

Message::Message(char *mess, int size)
    :mess(mess),size(size)
{
}

const char* Message::getMess() const
{
    return mess;
}

int Message::getSize() const
{
    return size;
}

