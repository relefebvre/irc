#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include "channel.h"

using namespace std;

class Channel ;

class Client
{
    int sock;
    string name;
    Channel *chanActif ; //Channel sur lequel le client peut envoyer et recevoir des messages

public:

    static int maxSock;

    Client(int sock);

    int getSock();

    string getName();
    void setName(string name);

    void setChan(Channel *newChan);
};

#endif // CLIENT_H
