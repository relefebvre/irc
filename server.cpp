#include "server.h"

#include <assert.h>
#include <string.h>
#include <iostream>



Server::Server()
{
    sock=socket(AF_INET,SOCK_STREAM,0);
    taille=sizeof(csin);
}

void Server::init(unsigned port)
{
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
}

int Server::conect()
{
    int test = accept(sock,(struct sockaddr *) &csin, &taille);
    if (test == -1)
        perror("Accept : ");
    return test;
}

int Server::getSock()
{
    return sock;
}

void Server::init_sockaddr(sockaddr_in *name, const char *hostname, uint16_t port)
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

void Server::closeSockServ()
{
    close(sock);
}


void Server::routine()
{
    while(1)
    {
        char buf[4096];
        unsigned tbuf = sizeof(buf);

        addAllSockets(&readfd);

        if ( select(Client::maxSock,&readfd, NULL, NULL, NULL) == -1) {
            perror("Select ");
            closeAllSockets();
            closeSockServ();
            exit(1);
        }

        if (FD_ISSET(getSock(), &readfd))
        {
            char nameClt[40];
            Client *client = new Client(conect());
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
                        Channel* chan = channelByName(name);
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


    serv.closeAllSockets();
    clients.erase(clients.begin(),clients.end());
    closeSockServ();
}

void Server::addAllSockets(fd_set *readfd)
{
    FD_ZERO(readfd);
    FD_SET(sock, readfd);

    for(list<Client*>::iterator i=clients.begin() ; i != clients.end() ; ++i)
    {
        printf("Ajout de la socket : %d\n",(*i)->getSock());
        FD_SET((*i)->getSock(), readfd);
    }
}



int Server::writeToClt(Message* mess, string nameClt)
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
  Ferme chaque socket de chaque client de la liste client.
 */

void Server::closeAllSockets()
{
    for(list<Client*>::iterator i=clients.begin() ; i != clients.end() ; ++i)
    {
        if (close((*i)->getSock()) == -1)
        {
            perror("Fermeture socket ");
            exit(1);
        }
    }
}


Channel* Server::channelByName(string chanName)
{
    map<string , Channel*>::const_iterator ch(chanMap.find(chanName));

    for(map<string , Channel*>::const_iterator it=chanMap.begin() ; it!=chanMap.end() ; it++)
        if (it == ch)
            return it->second;

    return NULL;
}

void Server::addChan(string chanName, Channel *chan)
{
    chanMap.insert(make_pair(chanName, chan));
}

