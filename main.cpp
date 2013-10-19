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
void broadcast(Message* mess, list<Client*> client);


/*
  Ajoute chaque socket de chauque client de la liste client,
  dans readfd.
 */








int main(int argc, char **argv)
{
    Server serv;
    unsigned port;

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
    serv.routine() ;




    return 0;
}

