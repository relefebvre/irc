#include "channel.h"

#include "client.h"
#include "string.h"
#include <unistd.h>
#include <regex.h>



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


int Channel::kickClt(const string motif)
{
    int nb=0;
    regex_t expr;

    if ( (regcomp(&expr, motif.c_str(),REG_EXTENDED)) == 0)
    {

        for (list<Client*>::iterator i=users.begin() ; i != users.end() ; ++i)
        {
            if ((regexec(&expr,(*i)->getName().c_str(),0,NULL,0)) == 0)
            {
                users.erase(i);
                ++nb;
            }
        }
    }
    return nb;
}

int Channel::addBanned(const string motif)
{
    int nb=0;
    regex_t expr;

    if ( (regcomp(&expr, motif.c_str(),REG_EXTENDED)) == 0)
    {
        for (list<Client*>::const_iterator i=users.begin() ; i != users.end() ; ++i)
            if ((regexec(&expr,(*i)->getName().c_str(),0,NULL,0)) == 0)
            {
                banned.push_back(*i);
                ++nb;
            }
    }
    return nb;
}


int Channel::setOp(const string nameClt)
{
    for (list<Client*>::const_iterator i=users.begin() ; i != users.end() ; ++i)
        if ((*i)->getName() == nameClt)
        {
            op=nameClt;
            return 0;
        }
    return -1;
}
