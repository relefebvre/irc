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
                Commande *cde,*cde1;

                cde = whatIsTrame((*i)->getSock());

                if ((cde1 = receive(cde,(*i)->getName())) == NULL)
                    writeToClt(cde->getError(),(*i)->getName());
                 else
                    send(cde1,(*i)->getName());
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


Commande *Server::receive(Commande *cde, const string nameClt)
{
            Channel *chan;
              list<Client*> cltSearch;
              list<Channel*> chanSearch;
              Commande *newCde;

    switch (cde->getCde()) {

    /*------/msg client message-----*/

    case (char)1 :
        if (cde->getNbArgs() != 2)
        {
            cde->setError("Le nombre d'arguments n'est pas correct",1);
            return NULL;
        }

        newCde = new Commande(cde->getIdCde(),(char)129);
        newCde->addArg(cde->getArg(1));
        newCde->addArg(cde->getArg(2));
        break;

    /*-----/msg channel message----*/

    case (char)2 :
        if (cde->getNbArgs() != 2)
        {
            cde->setError("Le nombre d'arguments n'est pas correct",2);
            return NULL;
        }

        chan = channelByName(cde->getArg(1).substr(1));

        if (chan == NULL)
            cde->setError("Channel inconnu",3);
        else
        {
            newCde = new Commande(cde->getIdCde(),(char)128);
            newCde->addArg(cde->getArg(1));
            newCde->addArg(nameClt);
            newCde->addArg(cde->getArg(2));
        }
        break;

    /*-----/who motif-----*/

    case (char)3 :
        if (cde->getNbArgs() != 1)
        {
            cde->setError("Le nombre d'arguments n'est pas correct",4);
            return NULL;
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

    case (char)4 :
        if (cde->getNbArgs() != 1)
        {
            cde->setError("Le nombre d'arguments n'est pas correct",5);
            return NULL;
        }
        chan = channelByName(cde->getArg(1));

        if(chan == NULL)
        {
             cde->setError("Le channel n'existe pas",6);
             return NULL;
        }

        cltSearch = chan->searchClt();

        for (list<Client*>::const_iterator it=cltSearch.begin() ; it!=cltSearch.end() ; ++it)
            writeToClt((*it)->getName()+"\n",nameClt);

        cltSearch.erase(cltSearch.begin(),cltSearch.end());
        break;

  /*-----/list [motif]----*/

    case (char)5 :
        if (cde->getNbArgs() == 0)
            cde->addArg("*");

        chanSearch = searchChan(cde->getArg(1));

        for (list<Channel*>::const_iterator it=chanSearch.begin() ; it!=chanSearch.end() ; ++it)
            writeToClt((*it)->getChanName()+" "+(*it)->getTopic()+"\n",nameClt);

        chanSearch.erase(chanSearch.begin(),chanSearch.end());
        break;

    /*-----/topic newTopic----*/

    case (char)6 :
        if (cde->getNbArgs() > 2)
        {
            cde->setError("Le nombre d'arguments n'est pas correct",7);
            return NULL;
        }

        chan = channelByName(cde->getArg(1));

        if(chan == NULL)
        {
             cde->setError("Le channel n'existe pas",8);
             return NULL;
        }

        if (cde->getNbArgs() == 0)
            writeToClt(chan->getTopic(),nameClt);
        else
        {
            newCde = new Commande(cde->getIdCde(),(char)131);
            newCde->addArg(chan->getChanName());
            newCde->addArg(cde->getArg(2));
            chan->setTopic(cde->getArg(2));
        }
        break;


  /*-----/kick channel motif-----*/

    case (char)7 :
        if (cde->getNbArgs() != 2)
        {
            cde->setError("Le nombre d'arguments n'est pas correct",9);
            return NULL;
        }

        chan = channelByName(cde->getArg(1));

        if(chan == NULL)
        {
             cde->setError("Le channel n'existe pas",10);
             return NULL;
        }

        cltSearch = chan->kickClt(cde->getArg(1));

        if (cltSearch.empty())
        {
            cde->setError("Le nick n'est pas sur ce channel",11);
            return NULL;
        }
        else
        {
            newCde = new Commande(cde->getIdCde(),(char)134);
            for (list<Client*>::const_iterator it=cltSearch.begin(); it!=cltSearch.end(); ++it)
                newCde->addArg((*it)->getName());
            newCde->addArg(nameClt);
        }
        break;


    /*------/ban channel motif-----*/

    case (char)8 :
        if (cde->getNbArgs() != 2)
        {
            cde->setError("Le nombre d'arguments n'est pas correct",12);
            return NULL;
        }

        chan = channelByName(cde->getArg(1));

        if(chan == NULL)
        {
             cde->setError("Le channel n'existe pas",13);
             return NULL;
        }

        if ((chan->addBanned(cde->getArg(2))) == 0)
        {
            cde->setError("Le nick n'est pas sur ce channel",14);
            return NULL;
        }
        else
        {
            newCde = new Commande(cde->getIdCde(),(char)135);
            newCde->addArg(cde->getArg(1));
            newCde->addArg("+"+cde->getArg(2));
        }
        break;


   /*-----/op channel nick------*/

    case (char)9 :
        if (cde->getNbArgs() != 2)
        {
            cde->setError("Le nombre d'arguments n'est pas correct",15);
            return NULL;
        }

        chan = channelByName(cde->getArg(1));

        if(chan == NULL)
        {
             cde->setError("Le channel n'existe pas",16);
             return NULL;
        }

        if ((chan->setOp(cde->getArg(2))) == -1)
        {
            cde->setError("Le nick n'est pas sur ce channel",17);
            return NULL;
        }
        break;



   /*-----/deop channel nick-----*/

    case (char)20 :
        if (cde->getNbArgs() != 2)
        {
            cde->setError("Le nombre d'arguments n'est pas correct",18);
            return NULL;
        }

        chan = channelByName(cde->getArg(1));

        if(chan == NULL)
        {
             cde->setError("Le channel n'existe pas",19);
             return NULL;
        }

        if ((chan->supprOp(cde->getArg(2))) == -1)
        {
            cde->setError("Le nick n'est pas sur ce channel",20);
            return NULL;
        }
        break;


  /*-----/join channel------*/

    case (char)21 :
        if (cde->getNbArgs() != 1)
        {
            cde->setError("Le nombre d'arguments n'est pas correct",21);
            return NULL;
        }

        chan = channelByName(cde->getArg(1));

        if (chan == NULL)
        {
            addChan(chan = new Channel(cde->getArg(1), nameClt));
            chan->addUser((searchClt(nameClt)).front());
            writeToClt("Vous êtes l'opérateur du channel "+chan->getChanName()+"\n",nameClt);
            /*!!!!!!!!!return!!!!!!!!!*/
        }

        if (chan->isBanned(nameClt))
        {
            cde->setError("Le client "+nameClt+" est banni du channel "+cde->getArg(1),22);
            return NULL;
        }

        chan->addUser((searchClt(nameClt)).front());

        newCde = new Commande(cde->getIdCde(),(char)137);
        newCde->addArg(cde->getArg(1));
        newCde->addArg(nameClt);
        break;

    /*------/nick newNick-----*/

    case (char)22 :
        if (cde->getNbArgs() != 1)
        {
            cde->setError("Le nombre d'arguments n'est pas correct",23);
            return NULL;
        }

        if (cde->getArg(1) == "")
        {
            cde->setError("Nick invalide",24);
            return NULL;
        }

        if (changeNameClt(nameClt,cde->getArg(1)) == -1)
        {
            cde->setError("Nom \""+cde->getArg(1)+"\" déja pris",25);
            return NULL;
        }
        else
        {
            newCde = new Commande(cde->getIdCde(),(char)132);
            newCde->addArg(nameClt);
            newCde->addArg(cde->getArg(1));
        }
        break;


   /*-----/leave channel----*/

    case (char)23 :
        if (cde->getNbArgs() != 1)
        {
            cde->setError("Le nombre d'arguments n'est pas correct",26);
            return NULL;
        }

        chan = channelByName(cde->getArg(1));

        if(chan == NULL)
        {
             cde->setError("Le channel n'existe pas",27);
             return NULL;
        }

        cltSearch = chan->kickClt(nameClt);
        if (cltSearch.empty())
        {
            cde->setError("Client inconnu",28);
            return NULL;
        }
        else
        {
            newCde = new Commande(cde->getIdCde(),(char)133);
            newCde->addArg(cde->getArg(1));
            newCde->addArg(nameClt);
        }
        break;


   /*-----/unban channel motif-----*/

    case (char)24 :
        if (cde->getNbArgs() != 2)
        {
            cde->setError("Le nombre d'arguments n'est pas correct",29);
            return NULL;
        }

        chan = channelByName(cde->getArg(1));

        if(chan == NULL)
        {
             cde->setError("Le channel n'existe pas",30);
             return NULL;
        }

        if ((chan->supprBanned(cde->getArg(2))) == 0)
        {
            cde->setError("Le nick n'est pas sur banni de ce channel",31);
            return NULL;
        }
        else
        {
            newCde = new Commande(cde->getIdCde(),(char)135);
            newCde->addArg(cde->getArg(1));
            newCde->addArg("-"+cde->getArg(2));
        }
        break;


    /*-----/banlist channel----*/

    case (char)25 :
        if (cde->getNbArgs() != 1)
        {
            cde->setError("Le nombre d'arguments n'est pas correct",32);
            return NULL;
        }

        chan = channelByName(cde->getArg(1));

        if(chan == NULL)
        {
             cde->setError("Le channel n'existe pas",33);
             return NULL;
        }

        cltSearch = chan->listBan();

        for (list<Client*>::const_iterator it=cltSearch.begin() ; it!=cltSearch.end() ; ++it)
            writeToClt((*it)->getName()+"\n",nameClt);

        cltSearch.erase(cltSearch.begin(),cltSearch.end());
        break;

    }
    return newCde;
}


void Server::send(Commande *cde,const string nameClt)
{
    Channel* chan;
    int i;

    switch (cde->getCde()) {

    case (char)128 :
        chan = channelByName(cde->getArg(1));
        chan->broadcast(cde->getArg(2)+" : "+cde->getArg(3));
        break;

    case (char)129 :
        writeToClt(cde->getArg(1)+" : "+cde->getArg(2),nameClt);
        break;

    case (char)130 :

        break;

    case (char)131 :
        chan = channelByName(cde->getArg(1));
        chan->broadcast("Nouveau Topic : "+cde->getArg(2));
        break;

    case (char)132 :
        writeToClt("Le client "+cde->getArg(1)+" devient "+cde->getArg(2),nameClt);
        break;

    case (char)133 :
        chan = channelByName(cde->getArg(1));
        chan->broadcast("L'utilisateur : "+cde->getArg(2)+" a quitté le channel");
        break;

    case (char)134 :
        chan = channelByName(cde->getArg(1));
        for (i=2 ; i<cde->getNbArgs() ; ++i)
            chan->broadcast("L'utilisateur : "+cde->getArg(i)+" a été kické du channel");
        break;

    case (char)135 :
        chan = channelByName(cde->getArg(1));
        chan->broadcast("Ban : "+cde->getArg(2));
        break;

    case (char)136 :
        break;

    case (char)137 :
        chan = channelByName(cde->getArg(1));
        chan->broadcast("L'utilisateur : "+cde->getArg(2)+" a rejoint le channel");
        break;
    }
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
    printf("%c\n",cde);

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
