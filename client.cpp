/*
  Auteurs : Ducros Alix & Lefebvre De Ladonchamps Rémi
  Fichier : client.cpp
  Fonctionnement : Classe Client.
 */

#include "client.h"

int Client::maxSock = 0;

Client::Client(int sock)
    :sock(sock)
{
    if (sock+1 > Client::maxSock)
        Client::maxSock = sock+1;
}

int Client::getSock()
{
    return this->sock;
}

string Client::getName()
{
    return this->name;
}

void Client::setName(string name)
{
    this->name = name;
}

void Client::setChan(Channel *newChan)
{
    chanActif = newChan ;
}
