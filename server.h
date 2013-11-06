#ifndef SERVER_H
#define SERVER_H


#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <map>

#include "channel.h"
#include "client.h"
#include "message.h"
#include "commande.h"



class Server
{
    int sock;
    struct sockaddr_in sin, csin;
    socklen_t taille;
    list<Channel*> channels ;
    list<Client*> clients;
    list<Message*> messages;
    fd_set readfd;



public:
    Server();
    void init(unsigned port);
    int conect();
    int getSock() const;

    void init_sockaddr (struct sockaddr_in *name, const char *hostname, uint16_t port);
    void closeSockServ();

    void addAllSockets(fd_set *readfd) ;
    void closeAllSockets() ;

    int writeToClt(const char *message, const string nameClt) const ;

    void routine();
    void closeAll();

    Commande* receive(Commande *cde, const string nameClt) ;
    void send(Commande *cde);

    //Méthode sur les chan

    Channel* channelByName(string chanName); //trouve un channel en fonction de son nom
    void addChan(Channel *chan);

    Commande* whatIsTrame(int sock);

    list<Client*> searchClt(const string motifClt) const;

    bool isClt(const string nameClt) const;
    Client* searchOneClt(const string nameClt) const;

    list<Channel*> searchChan(const string motifChan) const;

    int changeNameClt(const string nameClt, const string newName);


};

#endif // SERVER_H
