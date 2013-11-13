#ifndef CHANNEL_H
#define CHANNEL_H

#include <string>
#include <list>
#include "client.h"


using namespace std;

class Client ;


class Channel
{
private :
    string chanName ;
    string op ;     //Nom du client ayant les droits d'OP sur le channel
    string topic;

    list<Client*> users ;
    list<Client*> banned;

public:
    Channel(string chanName, string op);
    ~Channel();

    bool isOp(const string nameClt) const;
    int setOp(const string nameClt);
    int supprOp(const string nameClt);

    const string & getChanName() const;

    const string & getTopic() const;
    void setTopic(const string topic);

    void addUser(Client *) ;

    void broadcast(const char *message) ;

    list<Client*> searchClt() const;

    list<Client*> kickClt(const string motif);

    int addBanned(const string motif);
    int supprBanned(const string motif);
    bool isBanned(const string nameClt) const;
    list<Client*> listBan() const;

    bool isEmpty() const;
};

#endif // CHANNEL_H
