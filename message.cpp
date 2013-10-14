#include "message.h"

int Message::nbMess = 0;

Message::Message(string mess, string nameClt)
    :mess(mess), nameClt(nameClt)
{
    Message::nbMess++;
}

string Message::getMess()
{
    return mess;
}

string Message::getName()
{
    return nameClt;
}
