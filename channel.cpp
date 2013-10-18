#include "channel.h"

#include "client.h"
#include "string.h"
#include <unistd.h>



Channel::Channel(string chanName,string op)
    : chanName(chanName), op(op)
{
}

string Channel::getOpName()
{
    return op;
}


string Channel::getChanName()
{
    return chanName;
}



void Channel::broadcast(Message* mess)
{
    string message;

    message = mess->getMess() + " : " + mess->getName();
    for (list<Client*>::iterator i=users.begin() ; i != users.end() ; ++i)
    {
        write((*i)->getSock(), message.c_str(),strlen(message.c_str()));
    }
}

int Server::addUser(Client* newClt)
{
    users.push_back(newClt);
}
