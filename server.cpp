#include "server.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <inttypes.h>
#include <regex.h>



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

int Server::getSock() const
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
            int size;
            Client *client = new Client(conect());
            clients.push_back(client);
            printf("Client connecté\n");
            assert(write(client->getSock(),&tbuf,sizeof(tbuf)) != -1 );
            read(client->getSock(),&size,sizeof(size));
            read(client->getSock(),nameClt,size);
            nameClt[size]='\0';
            client->setName(nameClt);

        }

        for(list<Client*>::iterator i=clients.begin(); i != clients.end() ; ++i)
            if (FD_ISSET((*i)->getSock(), &readfd))
            {
                Commande *cde;

                cde = whatIsTrame((*i)->getSock());

                interpreter(cde,(*i)->getName());
            }

}

void Server::closeAll()
{
    closeAllSockets();
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



int Server::writeToClt(const string message, const string nameClt) const
{
    for (list<Client*>::const_iterator i=clients.begin() ; i != clients.end() ; ++i)
    {
        if ((*i)->getName() == nameClt)
        {
            cout<<"Ecriture : "<<message<<endl;
            write((*i)->getSock(), message.c_str(),strlen(message.c_str()));
            return 0;
        }
    }
    return -1;
}


void Server::interpreter(Commande *cde, const string nameClt)
{
            Channel *chan;
              list<Client*> cltSearch;
              list<Channel*> chanSearch;

    switch (cde->getCde()) {

    /*------/msg client message-----*/

    case 0x01 :
        if (cde->getNbArgs() != 2)
        {
            cde->setError("Le nombre d'arguments n'est pas correct");
            return;
        }

        if ((writeToClt(cde->getArg(2)+"\n",cde->getArg(1))) == -1)
            cde->setError("Le client désigné n'existe pas");
        break;

    /*-----/msg channel message----*/

    case 0x02 :
        if (cde->getNbArgs() != 2)
        {
            cde->setError("Le nombre d'arguments n'est pas correct");
            return;
        }

        chan = channelByName(cde->getArg(1).substr(1));

        if (chan == NULL)
            cde->setError("Channel inconnu");
        else
            chan->broadcast(cde->getArg(2));
        break;

    /*-----/who motif-----*/

    case 0x03 :
        if (cde->getNbArgs() != 1)
        {
            cde->setError("Le nombre d'arguments n'est pas correct");
            return;
        }
        cltSearch = searchClt(cde->getArg(1));

        for (list<Client*>::const_iterator it=cltSearch.begin() ; it!=cltSearch.end() ; ++it)
        {
            cout<<(*it)->getName()<<endl;
            writeToClt((*it)->getName()+"\n",nameClt);
        }
        cltSearch.erase(cltSearch.begin(),cltSearch.end());
        break;


   /*-----/who chanName-----*/

    case 0x04 :
        if (cde->getNbArgs() != 1)
        {
            cde->setError("Le nombre d'arguments n'est pas correct");
            return;
        }
        chan = channelByName(cde->getArg(1));

        if(chan == NULL)
        {
             cde->setError("Le channel n'existe pas");
             return;
        }

        cltSearch = chan->searchClt();

        for (list<Client*>::const_iterator it=cltSearch.begin() ; it!=cltSearch.end() ; ++it)
            writeToClt((*it)->getName()+"\n",nameClt);

        cltSearch.erase(cltSearch.begin(),cltSearch.end());
        break;

  /*-----/list [motif]----*/

    case 0x05 :
        if (cde->getNbArgs() == 0)
            cde->addArg("*");

        chanSearch = searchChan(cde->getArg(1));

        for (list<Channel*>::const_iterator it=chanSearch.begin() ; it!=chanSearch.end() ; ++it)
            writeToClt((*it)->getChanName()+" "+(*it)->getTopic()+"\n",nameClt);

        chanSearch.erase(chanSearch.begin(),chanSearch.end());
        break;

    /*-----/topic newTopic----*/

    case 0x06 :
        if (cde->getNbArgs() < 1 || cde->getNbArgs() > 2)
        {
            cde->setError("Le nombre d'arguments n'est pas correct");
            return;
        }

        chan = channelByName(cde->getArg(1));

        if(chan == NULL)
        {
             cde->setError("Le channel n'existe pas");
             return;
        }

        if (cde->getNbArgs() == 1)
            writeToClt(chan->getTopic(),nameClt);
        else
            chan->setTopic(cde->getArg(2));
        break;


  /*-----/kick channel motif-----*/

    case 0x07 :
        if (cde->getNbArgs() != 2)
        {
            cde->setError("Le nombre d'arguments n'est pas correct");
            return;
        }

        chan = channelByName(cde->getArg(1));

        if(chan == NULL)
        {
             cde->setError("Le channel n'existe pas");
             return;
        }

        if ((chan->kickClt(cde->getArg(1))) == 0)
        {
            cde->setError("Le nick n'est pas sur ce channel");
            return;
        }
        break;


    /*------/ban channel motif-----*/

    case 0x08 :
        if (cde->getNbArgs() != 2)
        {
            cde->setError("Le nombre d'arguments n'est pas correct");
            return;
        }

        chan = channelByName(cde->getArg(1));

        if(chan == NULL)
        {
             cde->setError("Le channel n'existe pas");
             return;
        }

        if ((chan->addBanned(cde->getArg(2))) == 0)
        {
            cde->setError("Le nick n'est pas sur ce channel");
            return;
        }
        break;


   /*-----/op channel nick------*/

    case 0x09 :
        if (cde->getNbArgs() != 2)
        {
            cde->setError("Le nombre d'arguments n'est pas correct");
            return;
        }

        chan = channelByName(cde->getArg(1));

        if(chan == NULL)
        {
             cde->setError("Le channel n'existe pas");
             return;
        }

        if ((chan->setOp(cde->getArg(2))) == -1)
        {
            cde->setError("Le nick n'est pas sur ce channel");
            return;
        }
        break;



   /*-----/deop channel nick-----*/

    case 0x20 :
        if (cde->getNbArgs() != 2)
        {
            cde->setError("Le nombre d'arguments n'est pas correct");
            return;
        }

        chan = channelByName(cde->getArg(1));

        if(chan == NULL)
        {
             cde->setError("Le channel n'existe pas");
             return;
        }

        if ((chan->supprOp(cde->getArg(2))) == -1)
        {
            cde->setError("Le nick n'est pas sur ce channel");
            return;
        }
        break;


  /*-----/join channel------*/

    case 0x21 :
        if (cde->getNbArgs() != 1)
        {
            cde->setError("Le nombre d'arguments n'est pas correct");
            return;
        }

        chan = channelByName(cde->getArg(1));

        if (chan == NULL)
        {
            addChan(chan = new Channel(cde->getArg(1), nameClt));
            chan->addUser((searchClt(nameClt)).front());
            writeToClt("Vous êtes l'opérateur du channel "+chan->getChanName()+"\n",nameClt);
            return;
        }

        if (chan->isBanned(nameClt))
        {
            cde->setError("Le client "+nameClt+" est banni du channel "+cde->getArg(1));
            return;
        }

        chan->addUser((searchClt(nameClt)).front());
        writeToClt("Client "+nameClt+" ajouté au channel "+cde->getArg(1)+"\n",nameClt);
        break;

    /*------/nick newNick-----*/

    case 0x22 :
        if (cde->getNbArgs() != 1)
        {
            cde->setError("Le nombre d'arguments n'est pas correct");
            return;
        }

        if (cde->getArg(1) == "")
        {
            cde->setError("Nick invalide");
            return;
        }

        if (changeNameClt(nameClt,cde->getArg(1)) == -1)
        {
            cde->setError("Nom \""+cde->getArg(1)+"\" déja pris");
            return;
        }
        break;


   /*-----/leave channel----*/

    case 0x23 :
        if (cde->getNbArgs() != 1)
        {
            cde->setError("Le nombre d'arguments n'est pas correct");
            return;
        }

        chan = channelByName(cde->getArg(1));

        if(chan == NULL)
        {
             cde->setError("Le channel n'existe pas");
             return;
        }

        if ((chan->kickClt(nameClt)) == 0)
        {
            cde->setError("Client inconnu");
            return;
        }
        break;


   /*-----/unban channel motif-----*/

    case 0x24 :
        if (cde->getNbArgs() != 2)
        {
            cde->setError("Le nombre d'arguments n'est pas correct");
            return;
        }

        chan = channelByName(cde->getArg(1));

        if(chan == NULL)
        {
             cde->setError("Le channel n'existe pas");
             return;
        }

        if ((chan->supprBanned(cde->getArg(2))) == 0)
        {
            cde->setError("Le nick n'est pas sur banni de ce channel");
            return;
        }
        break;


    /*-----/banlist channel----*/

    case 0x25 :
        if (cde->getNbArgs() != 1)
        {
            cde->setError("Le nombre d'arguments n'est pas correct");
            return;
        }

        chan = channelByName(cde->getArg(1));

        if(chan == NULL)
        {
             cde->setError("Le channel n'existe pas");
             return;
        }

        cltSearch = chan->listBan();

        for (list<Client*>::const_iterator it=cltSearch.begin() ; it!=cltSearch.end() ; ++it)
            writeToClt((*it)->getName()+"\n",nameClt);

        cltSearch.erase(cltSearch.begin(),cltSearch.end());
        break;

    }
    return;
}






void Server::closeAllSockets()
{/*
  Ferme chaque socket de chaque client de la liste client.
 */
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
    for(list<Channel*>::const_iterator it=channels.begin() ; it!=channels.end() ; it++)
        if ((*it)->getChanName() == chanName)
            return (*it);

    return NULL;
}

void Server::addChan(Channel *chan)
{
    channels.push_back(chan);
}


Commande* Server::whatIsTrame(int sock)
{
    uint16_t idCde, sizeTrame;
    //unsigned int cde;
    char cde, buf[4096];

    read(sock,&buf,sizeof(sizeTrame));
    sscanf(buf,"%hd",&sizeTrame);
    read(sock,&buf,sizeof(idCde));
    sscanf(buf,"%hd",&idCde);
    read(sock,&buf,sizeof(cde));
    sscanf(buf,"%c",&cde);


    printf("%hd\n",sizeTrame);
    printf("%hd\n",idCde);
    printf("%x\n",cde);

    int tbuf=sizeTrame-sizeof(idCde)-sizeof(cde);

    read(sock,buf,tbuf);

    cout<<"Buf : "<<buf;

    Commande *Cde = new Commande(idCde,cde);


    char *arg;
    arg = strtok(buf,"\n");

    while (arg != NULL)
    {
        Cde->addArg(arg);
        arg = strtok(NULL,"\n");
    }

    return Cde;
}


list<Client*> Server::searchClt(const string motifClt) const
{
    regex_t expr;
    list<Client*> cltSearch;

    if ( (regcomp(&expr, motifClt.c_str(),REG_EXTENDED)) == 0)
    {

            for (list<Client*>::const_iterator it=clients.begin() ; it!=clients.end() ; ++it)
                if ((regexec(&expr,(*it)->getName().c_str(),0,NULL,0)) == 0)
                    cltSearch.push_back(*it);

    }

    return cltSearch;
}


list<Channel*> Server::searchChan(const string motifChan) const
{
    regex_t expr;
    list<Channel*> chanSearch;

    if ( (regcomp(&expr, motifChan.c_str(),REG_EXTENDED)) == 0)
    {

            for(list<Channel*>::const_iterator it=channels.begin() ; it!=channels.end() ; it++)
                if ((regexec(&expr,(*it)->getChanName().c_str(),0,NULL,0)) == 0)
                    chanSearch.push_back(*it);


    }

    return chanSearch;
}


int Server::changeNameClt(const string nameClt ,const string newName)
{
    for (list<Client*>::const_iterator it=clients.begin() ; it!=clients.end() ; ++it)
    {
        if ((*it)->getName() == newName)
            return -1;
        if ((*it)->getName() == nameClt)
        {
            (*it)->setName(newName);
            return 0;
        }
    }
    return -1;
}
