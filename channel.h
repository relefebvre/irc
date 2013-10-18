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
    const string & getOpName() const;
    const string & getChanName() const;
    void addUser(Client *) ;
    void broadcast(const string message) ;

};

#endif // CHANNEL_H
