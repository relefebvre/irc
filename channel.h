#ifndef CHANNEL_H
#define CHANNEL_H

#include <string>
#include <list>
#include "client.h"
#include "message.h"


using namespace std;

class Client ;


class Channel
{
private :
    string chanName ;
    string op ;     //Nom du client ayant les droits d'OP sur le channel

    list<Client *> users ;

public:
    Channel(string chanName, string op);
    string getOpName();
    string getChanName();
    int addUser(Client *) ;
    void broadcast(Message *mess) ;

};

#endif // CHANNEL_H
