#ifndef SERVER_H
#define SERVER_H


#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <map>

#include "channel.h"
#include "client.h"
#include "message.h"



class Server
{
    int sock;
    struct sockaddr_in sin, csin;
    socklen_t taille;
    map<string , Channel*> chanMap ;
    list<Client*> clients;
    list<Message*> messages;
    fd_set readfd;



public:
    Server();
    void init(unsigned port);
    int conect();
    int getSock();
    void init_sockaddr (struct sockaddr_in *name, const char *hostname, uint16_t port);
    void closeSockServ();
    void addAllSockets(fd_set *readfd) ;
    void closeAllSockets() ;
    void routine();
    //Méthode sur les chan
    Channel* channelByName(string chanName); //trouve un channel en fonction de son nom
    void addChan(string chanName, Channel *chan);


};

#endif // SERVER_H
