/*
  Auteurs : Ducros Alix & Lefebvre De Ladonchamps Rémi
  Fichier : client.cpp
  Fonctionnement : Classe Client.
 */

#include "client.h"

int Client::maxSock = 0;

Client::Client(int newSock, string newName)
{
    this->sock = newSock;
    if (newSock+1 > Client::maxSock)
        Client::maxSock = newSock+1;

    this->name = newName;
}

int Client::getSock()
{
    return this->sock;
}

string Client::getName()
{
    return this->name;
}

void Client::setChan(Channel *newChan)
{
    chanActif = newChan ;
}
