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
#include "server.h"

using namespace std;


void addAllSockets(list<Client*> client, fd_set *readfd, int sock);
void closeAllSockets(list<Client*> client);


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
    for (list<Client*>::iterator i=client.begin() ; i != client.end() ; ++i)
    {
        write((*i)->getSock(), message.c_str(),strlen(message.c_str()));
    }
}


/*
  Argument : port : si absent 1025 par défaut.
 */

int main(int argc, char **argv)
{
    Server serv;
    unsigned port;
    fd_set readfd;
    list<Client*> clients;
    list<Message*> messages;

    if(argc >= 2 )
        if ( sscanf(argv[1],"%u",&port) != 1  )
        {
            fprintf(stderr,"Numéro de port invalide\n");
            serv.closeSockServ();
            exit(1);
        }

    if (argc < 2)
    {
        port = 1025 ;
    }

    cout << "Port ouvert : " << port << endl;

    serv.init(port);

    if (serv.getSock()+1 > Client::maxSock)
        Client::maxSock = serv.getSock()+1;

    while(1)
    {
        char buf[4096];
        unsigned tbuf = sizeof(buf);

        addAllSockets(clients, &readfd, serv.getSock());

        if ( select(Client::maxSock,&readfd, NULL, NULL, NULL) == -1) {
            perror("Select ");
            closeAllSockets(clients);
            serv.closeSockServ();
            exit(1);
        }

        if (FD_ISSET(serv.getSock(), &readfd))
        {
            Client *client = new Client(serv.conect(),"default");
            clients.push_back(client);
            printf("Client connecté\n");
            assert(write(client->getSock(),&tbuf,sizeof(tbuf)) != -1 );

        }

        for(list<Client*>::iterator i=clients.begin(); i != clients.end() ; ++i)
            if (FD_ISSET((*i)->getSock(), &readfd))
            {
                read((*i)->getSock(),buf,sizeof(buf));
                if(buf[strlen(buf)-1]=='\n')
                    buf[strlen(buf)-1]='\0';
                cout << "Message lu : "<< buf << endl;
                Message *mess=new Message(buf,(*i)->getName());
                messages.push_back(mess);
                broadcast(mess,clients);
            }

    }

    closeAllSockets(clients);
    clients.erase(clients.begin(),clients.end());
    serv.closeSockServ();

    return 0;
}

