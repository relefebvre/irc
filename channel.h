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
    string topic;

    list<Client *> users ;
    list<Client*> banned;

public:
    Channel(string chanName, string op);
    const string & getOpName() const;
    const string & getChanName() const;
    const string & getTopic() const;
    void setTopic(const string topic);
    void addUser(Client *) ;
    void broadcast(const string message) ;

    list<Client*> searchClt() const;

    int kickClt(const string motif);

    int addBanned(const string motif);

    int setOp(const string nameClt);

};

#endif // CHANNEL_H
