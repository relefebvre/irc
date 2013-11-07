/*
  Auteurs : Ducros Alix & Lefebvre De Ladonchamps Rémi
  Fichier : main.cpp
  Fonctionnement : Serveur IRC.
 */

#include <iostream>
#include <stdio.h>
#include "server.h"

using namespace std;



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


    Commande::initErrors();

    serv.init(port);

    if (serv.getSock()+1 > Client::maxSock)
        Client::maxSock = serv.getSock()+1;

    while(serv.routine() != "quit");



    return 0;
}

