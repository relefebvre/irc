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



void broadcast(Message* mess, list<Client*> clients)
{
    string message;

    message = mess->getMess() + " : " + mess->getName();
    for (list<Client*>::iterator i=clients.begin() ; i != clients.end() ; ++i)
    {
        write((*i)->getSock(), message.c_str(),strlen(message.c_str()));
    }
}

int writeToClt(Message* mess, list<Client*> clients, string nameClt)
{
    string message = mess->getMess();

    for (list<Client*>::iterator i=clients.begin() ; i != clients.end() ; ++i)
    {
        if ((*i)->getName() == nameClt)
        {
            write((*i)->getSock(), message.c_str(),strlen(message.c_str()));
            return 0;
        }
    }
    return -1;
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
            char nameClt[40];
            Client *client = new Client(serv.conect());
            clients.push_back(client);
            printf("Client connecté\n");
            assert(write(client->getSock(),&tbuf,sizeof(tbuf)) != -1 );
            read(client->getSock(),nameClt,sizeof(nameClt));
            client->setName(nameClt);

        }

        for(list<Client*>::iterator i=clients.begin(); i != clients.end() ; ++i)
            if (FD_ISSET((*i)->getSock(), &readfd))
            {
                read((*i)->getSock(),buf,sizeof(buf));

                if (buf[0] == '/')
                {
                    char cmd[10];

                    sscanf(buf,"%*c%s%*c%[^\n]",cmd,buf);

                    if (strcmp("join",cmd)==0)
                    {
                        string name(buf);
                        Channel* chan = serv.channelByName(name);
                        cout << "Création du channel : "<<name<<endl;
                        if(chan == NULL)
                        {

                            chan = new Channel(name, (*i)->getName());
                            serv.addChan(name,chan);
                            cout << "Ajout du channel : "<<name<< " à la map."<<endl;
                            cout << "Oppérateur du channel : "<<chan->getOpName()<<endl;
                            string message = "Vous êtes l'oppérateur du channel " + chan->getChanName();
                            write((*i)->getSock(),message.c_str(),strlen(message.c_str()));
                        }
                        else
                        {
                            cout << "Channel déjà existant"<<endl;
                        }

                    }

                    if (strcmp("mess",cmd) == 0)
                    {
                        char nameClt[40];
                        sscanf(buf,"%s%*c%[^\n]",nameClt,buf);
                        Message *mess=new Message(buf,(*i)->getName());
                        if(writeToClt(mess,clients,nameClt) == -1)
                        {
                            string SnameClt(nameClt);
                            string message = "Le client "+SnameClt+" n'est pas connecté";
                            write((*i)->getSock(),message.c_str(),strlen(message.c_str()));
                        }
                        else
                            cout<<"Message envoyé"<<endl;
                    }

                    cout << "Commande : "<<cmd << " reste : "<<buf <<endl;
                }
                else
                {

                    if(buf[strlen(buf)-1]=='\n')
                        buf[strlen(buf)-1]='\0';

                    cout << "Message lu : "<< buf << endl;
                    Message *mess=new Message(buf,(*i)->getName());
                    messages.push_back(mess);
                    //broadcast(mess,clients);
                }
            }

    }

    closeAllSockets(clients);
    clients.erase(clients.begin(),clients.end());
    serv.closeSockServ();

    return 0;
}

