#include "channel.h"

#include "client.h"
#include "string.h"
#include <unistd.h>



Channel::Channel(string chanName,string op)
    : chanName(chanName), op(op)
{
    topic="Default Topic";
}

const string &Channel::getOpName() const
{
    return op;
}


const string &Channel::getChanName() const
{
    return chanName;
}

const string &Channel::getTopic() const
{
    return topic;
}

void Channel::setTopic(const string topic)
{
    this->topic = topic;
}


void Channel::broadcast(const string message)
{
    for (list<Client*>::iterator i=users.begin() ; i != users.end() ; ++i)
    {
        write((*i)->getSock(), message.c_str(),strlen(message.c_str()));
    }
}

void Channel::addUser(Client* newClt)
{
    users.push_back(newClt);
}

list<Client*> Channel::searchClt() const
{
    return users;
}
