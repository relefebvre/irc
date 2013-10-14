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

        list<Client *> users ;

    public:
        Channel(string chanName, string op);

};

#endif // CHANNEL_H
