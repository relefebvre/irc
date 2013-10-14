#include "channel.h"

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
