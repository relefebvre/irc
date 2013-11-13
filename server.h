#ifndef SERVER_H
#define SERVER_H

/*
 *      Classe Serveur
 *      Stocke
 *          - La socket Serveur
 *          - La liste des Channels
 *          - La liste des Clients connectés
 *
 */



#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include "channel.h"
#include "client.h"
#include "commande.h"



class Server
{
    int sock;
    struct sockaddr_in sin, csin;
    socklen_t taille;
    list<Channel*> channels ;
    list<Client*> clients;
    fd_set readfd;

public:
    Server();
    ~Server();

    void init(unsigned port);

    int connect();

    int getSock() const;

    void init_sockaddr (struct sockaddr_in *name, const char *hostname, uint16_t port);

    void closeSockServ();

    void addAllSockets(fd_set *readfd) ;
    void closeAllSockets() ;

    int writeToClt(const char *message, const string nameClt) const ;

    const string routine();
    void closeAll();

    Commande* receive(Commande *cde, const string nameClt) ;
    void send(Commande *cde);

    Channel* channelByName(string chanName);

    void addChan(Channel *chan);

    Commande* whatIsTrame(int sock);

    list<Client*> searchClt(const string motifClt) const;

    bool isClt(const string nameClt) const;

    list<Channel*> searchChan(const string motifChan) const;

    int changeNameClt(const string nameClt, const string newName);

    void broadcast(const char *message) ;
};

#endif // SERVER_H
