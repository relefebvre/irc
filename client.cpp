/*
  Auteurs : Ducros Alix & Lefebvre De Ladonchamps Rémi
  Fichier : client.cpp
  Fonctionnement : Classe Client.
 */

#include "client.h"
#include <sstream>

int Client::maxSock = 0;

Client::Client(int sock)
    :sock(sock)
{
    if (sock+1 > Client::maxSock)
        Client::maxSock = sock+1;
    ostringstream tmp;
    tmp << "Anonyme" << sock;
    name= tmp.str();
}

const int &Client::getSock() const
{
    return this->sock;
}

const string & Client::getName() const
{
    return this->name;
}

void Client::setName(const string name)
{
    this->name = name;
}

