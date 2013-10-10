/*
  Auteurs : Ducros Alix & Lefebvre De Ladonchamps Rémi
  Fichier : main.cpp
  Fonctionnement : Serveur IRC.
 */
#include <cassert>

#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <netdb.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <list>
#include <iterator>
#include "client.h"
#include "message.h"

using namespace std;


void init_sockaddr (struct sockaddr_in *name, const char *hostname, uint16_t port);
void addAllSockets(list<Client*> client, fd_set *readfd, int sock);
void closeAllSockets(list<Client*> client);


/*Initialisation de la socket name*/

void init_sockaddr (struct sockaddr_in *name,
                    const char *hostname,
                    uint16_t port)
{
    struct hostent *hostinfo;

    name->sin_family = AF_INET;   /* Adresses IPV4 (Internet) */
    name->sin_port = htons (port); /* On gère le little/big Endian */
    hostinfo = gethostbyname (hostname); /* appeler les fonctions de résolution de nom de la libC */
    if (hostinfo == NULL) /* Si ne peut transformer le nom de la machine en adresse IP */
    {
        fprintf (stderr, "Unknown host %s.\n", hostname);
        exit (EXIT_FAILURE);
    }
    name->sin_addr = *(struct in_addr *) hostinfo->h_addr_list[0]; /* Le dernier champs de la structure est garni */
}


/*
  Ajoute chaque socket de chauque client de la liste client,
  dans readfd.
 */

void addAllSockets(list<Client*> client, fd_set *readfd, int sock)
{
    FD_ZERO(readfd);
    FD_SET(sock, readfd);

    for(list<Client*>::iterator i=client.begin() ; i != client.end() ; ++i)
    {
        printf("Ajout de la socket : %d\n",(*i)->getSock());
        FD_SET((*i)->getSock(), readfd);
    }
}


/*
  Ferme chaque socket de chaque client de la liste client.
 */

void closeAllSockets(list<Client*> client)
{
    for(list<Client*>::iterator i=client.begin() ; i != client.end() ; ++i)
    {
        if (close((*i)->getSock()) == -1)
        {
            perror("Fermeture socket ");
            exit(1);
        }
    }
}



void broadcast(Message* mess, list<Client*> client)
{
    string message;

    message = mess->getMess() + " : " + mess->getName();
    message="Yoooooooo";
    for (list<Client*>::iterator i=client.begin() ; i != client.end() ; ++i)
    {
        cout << "test mess : " << message<<endl;
        write((*i)->getSock(), message.c_str(),strlen(message.c_str()));
    }
}


/*
  Argument : port : si absent 1025 par défaut.
 */

int main(int argc, char **argv)
{
    fprintf(stderr,"Numéro de port invalide\n");

    unsigned port;
    fd_set readfd;
    struct sockaddr_in sin, csin;
    int sock=socket(AF_INET,SOCK_STREAM,0);
    socklen_t taille=sizeof(csin);
    list<Client*> clients;
    list<Message*> messages;

    if(argc >= 2 )
        if ( sscanf(argv[1],"%u",&port) != 1  )
        {
            fprintf(stderr,"Numéro de port invalide\n");
            close(sock);
            exit(1);
        }

    if (argc < 2)
    {
        port = 1025 ;
    }

    cout << "Port ouvert : " << port << endl;

    if (sock+1 > Client::maxSock)
        Client::maxSock = sock+1;

    init_sockaddr(&sin,"0.0.0.0",port);

    if (bind(sock,(struct sockaddr *) &sin, sizeof(sin)) != 0)
    {
        perror("Erreur lors du bind de la socket") ;
        close(sock) ;
        exit(1) ;
    }

    if(listen(sock, 5) != 0)
    {
        perror("Listen non réussi ") ;
        close(sock) ;
        exit(1) ;
    }

    while(1)
    {

        addAllSockets(clients, &readfd, sock);


        if ( select(Client::maxSock,&readfd, NULL, NULL, NULL) == -1) {
            perror("Select ");
            closeAllSockets(clients);
            close(sock);
            exit(1);
        }

        if (FD_ISSET(sock, &readfd))
        {
            char buf[4096];
            unsigned tbuf = sizeof(buf);
            Client *client = new Client(accept(sock,(struct sockaddr *) &csin, &taille),"default");

            clients.push_back(client);
            printf("Client connecté\n");
            assert(write(client->getSock(),&tbuf,sizeof(tbuf)) != -1 );

        }

        for(list<Client*>::iterator i=clients.begin(); i != clients.end() ; ++i)
            if (FD_ISSET((*i)->getSock(), &readfd))
            {
                char buf[4096];

                read((*i)->getSock(),buf,sizeof(buf));
                cout << buf << (*i)->getName();
                Message *mess=new Message(buf,(*i)->getName());
                messages.push_back(mess);
                broadcast(mess,clients);
            }

    }

    closeAllSockets(clients);
    clients.erase(clients.begin(),clients.end());
    close(sock);

    return 0;
}

