#include "server.h"
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <regex.h>
#include <cerrno>



Server::Server()
{
    sock=socket(AF_INET,SOCK_STREAM,0);
    taille=sizeof(csin);
}

Server::~Server()
{
    for(list<Client*>::iterator it=clients.begin() ; it!=clients.end() ; ++it)
        delete *it;

    for(list<Channel*>::iterator it=channels.begin() ; it!=channels.end() ; ++it)
        delete *it;

    clients.erase(clients.begin(), clients.end());
    channels.erase(channels.begin(), channels.end());

    closeAll();
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


const string Server::routine()
{

    addAllSockets(&readfd);

    if ( select(Client::maxSock,&readfd, NULL, NULL, NULL) == -1) {
        perror("Select ");
        closeAllSockets();
        closeSockServ();
        exit(1);
    }

    if (FD_ISSET(getSock(), &readfd))
    {
        Client *client = new Client(conect());
        clients.push_back(client);
        printf("Client connecté\n");
    }

    if (FD_ISSET(0, &readfd))
    {
        char buf[4096];
        int nbLu;

        fflush(0);
        nbLu = read(0,buf,sizeof(buf));
        buf[nbLu-1]='\0';

        if (strcmp(buf,"quit") == 0)
        {
            Commande *cde = new Commande(0,136);
            cde->addArg("Déconnexion immminente !");
            send(cde);
            sleep(10);
            return "quit";
        }

    }

    for(list<Client*>::iterator i=clients.begin(); i != clients.end() ; ++i)
        if (FD_ISSET((*i)->getSock(), &readfd))
        {
            Commande *cde,*cde1;

            cde = whatIsTrame((*i)->getSock());

            cde1 = receive(cde,(*i)->getName());

            send(cde1);
        }

    return "continue";

}

void Server::closeAll()
{
    closeAllSockets();
    closeSockServ();
}

void Server::addAllSockets(fd_set *readfd)
{
    FD_ZERO(readfd);
    FD_SET(sock, readfd);
    FD_SET(0, readfd);

    for(list<Client*>::iterator i=clients.begin() ; i != clients.end() ; ++i)
    {
        printf("Ajout de la socket : %d\n",(*i)->getSock());
        FD_SET((*i)->getSock(), readfd);
    }
}



int Server::writeToClt(const char *message, const string nameClt) const
{
    uint16_t B ;
    uint16_t *taille = &B ;
    memcpy(taille, message,sizeof(B)) ;


    for (list<Client*>::const_iterator i=clients.begin() ; i != clients.end() ; ++i)
    {
        if ((*i)->getName() == nameClt)
        {
            write((*i)->getSock(), message,*taille+2);
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

    case 1 :
        newCde = new Commande(cde->getIdCde(),(char)129);

        if (cde->getNbArgs() != 2)
        {
            newCde->setError("",253,nameClt);
            break;
        }


        if (!isClt(cde->getArg(1)))
        {
            newCde->setError("le client désigné n'existe pas",254,nameClt);
            break;
        }

        newCde->setDest(cde->getArg(1));
        newCde->addArg(nameClt);
        newCde->addArg(cde->getArg(2));
        break;

        /*-----/msg channel message----*/

    case 2 :
        newCde = new Commande(cde->getIdCde(),(char)128);

        if (cde->getNbArgs() != 2)
        {
            newCde->setError("",253,nameClt);
            break;
        }

        /*chan = channelByName(cde->getArg(1).substr(1));   Si  #devant le nom de channel */
        chan = channelByName(cde->getArg(1));

        if (chan == NULL)
        {
            newCde->setError("Le channel n'existe pas",254,nameClt);
            break;
        }

        newCde->setDest(cde->getArg(1));
        newCde->addArg(cde->getArg(1));
        newCde->addArg(nameClt);
        newCde->addArg(cde->getArg(2));

        break;

        /*-----/who motif-----*/

    case 3 :
        newCde = new Commande(cde->getIdCde(),(char)129);

        if (cde->getNbArgs() != 1)
        {
            cde->setError("",253,nameClt);
            break;
        }

        newCde->setDest(nameClt);

        cltSearch = searchClt(cde->getArg(1));
        //Erreur motif, modifier fonction//

        newCde->addArg("Server");

        for (list<Client*>::const_iterator it=cltSearch.begin() ; it!=cltSearch.end() ; ++it)
            newCde->addArg((*it)->getName());

        cltSearch.erase(cltSearch.begin(),cltSearch.end());
        break;


        /*-----/who chanName-----*/

    case 4 :
        newCde = new Commande(cde->getIdCde(),(char)129);

        if (cde->getNbArgs() != 1)
        {
            newCde->setError("",253,nameClt);
            break;
        }

        chan = channelByName(cde->getArg(1));

        if(chan == NULL)
        {
            newCde->setError("Le channel n'existe pas",254,nameClt);
            break;
        }

        newCde->setDest(nameClt);

        cltSearch = chan->searchClt();

        newCde->addArg("Server");

        for (list<Client*>::const_iterator it=cltSearch.begin() ; it!=cltSearch.end() ; ++it)
            newCde->addArg((*it)->getName());

        cltSearch.erase(cltSearch.begin(),cltSearch.end());
        break;

        /*-----/list [motif]----*/

    case 5 :
        if (cde->getNbArgs() == 0)
            cde->addArg("");

        newCde = new Commande(cde->getIdCde(),(char)129);

        newCde->setDest(nameClt);

        chanSearch = searchChan(cde->getArg(1));
        //Erreur motif, modifier fonction//

        newCde->addArg("Server");

        for (list<Channel*>::const_iterator it=chanSearch.begin() ; it!=chanSearch.end() ; ++it)
            newCde->addArg((*it)->getChanName());

        chanSearch.erase(chanSearch.begin(),chanSearch.end());
        break;

        /*-----/topic channel [newTopic]----*/

    case 6 :
        newCde = new Commande(cde->getIdCde(),(char)131);

        if (cde->getNbArgs() > 2)
        {
            newCde->setError("",253,nameClt);
            break;
        }

        chan = channelByName(cde->getArg(1));

        if(chan == NULL)
        {
            newCde->setError("Le channel n'existe pas",254,nameClt);
            break;
        }

        newCde->setDest(cde->getArg(1));

        if (cde->getNbArgs() == 1)
        {
            newCde->addArg(chan->getTopic());
        }
        else
        {
            newCde->addArg(chan->getChanName());
            newCde->addArg(cde->getArg(2));
            chan->setTopic(cde->getArg(2));
        }
        break;


        /*-----/kick channel motif-----*/

    case 7 :
        newCde = new Commande(cde->getIdCde(),(char)134);

        if (cde->getNbArgs() != 2)
        {
            newCde->setError("",253,nameClt);
            break;
        }

        chan = channelByName(cde->getArg(1));

        if(chan == NULL)
        {
            newCde->setError("Le channel n'existe pas",254,nameClt);
            break;
        }

        newCde->setDest(cde->getArg(1));

        cltSearch = chan->kickClt(cde->getArg(1));

        if (cltSearch.empty())
            newCde->setError("Le client n'existe pas",254,nameClt);
        else
        {

            newCde->addArg(cde->getArg(1));
            for (list<Client*>::const_iterator it=cltSearch.begin(); it!=cltSearch.end(); ++it)
                newCde->addArg((*it)->getName());
            newCde->addArg(nameClt);
        }

        cltSearch.erase(cltSearch.begin(),cltSearch.end());

        if (chan->isEmpty())
            chan->~Channel();

        break;


        /*------/ban channel motif-----*/

    case 8 :
        newCde = new Commande(cde->getIdCde(),(char)135);

        if (cde->getNbArgs() != 2)
        {
            newCde->setError("",253,nameClt);
            break;
        }

        chan = channelByName(cde->getArg(1));

        if(chan == NULL)
        {
            newCde->setError("Le channel n'existe pas",254,nameClt);
            break;
        }

        newCde->setDest(cde->getArg(1));

        if ((chan->addBanned(cde->getArg(2))) == 0)
            newCde->setError("Le nick n'est pas sur ce channel",254,nameClt);
        else
        {

            newCde->addArg(cde->getArg(1));
            newCde->addArg("+");
            newCde->addArg(cde->getArg(2));
        }
        break;


        /*-----/op channel nick------*/

    case 9 :
        newCde = new Commande(cde->getIdCde(),(char)135);

        if (cde->getNbArgs() != 2)
        {
            newCde->setError("",253,nameClt);
            break;
        }

        chan = channelByName(cde->getArg(1));

        if(chan == NULL)
        {
            newCde->setError("Le channel n'existe pas",254,nameClt);
            return NULL;
        }

        if ((chan->setOp(cde->getArg(2))) == -1)
        {
            newCde->setError("Le nick n'est pas sur ce channel",254,nameClt);
            break;
        }

        newCde->setDest(cde->getArg(1));

        newCde->addArg(cde->getArg(2));
        newCde->addArg(cde->getArg(1));
        newCde->addArg("o");
        break;



        /*-----/deop channel nick-----*/

    case 20 :
        newCde = new Commande(cde->getIdCde(),(char)135);

        if (cde->getNbArgs() != 2)
        {
            newCde->setError("",253,nameClt);
            break;
        }

        chan = channelByName(cde->getArg(1));

        if(chan == NULL)
        {
            newCde->setError("Le channel n'existe pas",254,nameClt);
            return NULL;
        }

        if ((chan->supprOp(cde->getArg(2))) == -1)
        {
            newCde->setError("Le nick n'est pas sur ce channel",254,nameClt);
            break;
        }

        newCde->setDest(cde->getArg(1));

        newCde->addArg(cde->getArg(2));
        newCde->addArg(cde->getArg(1));
        newCde->addArg("");
        break;


        /*-----/join channel------*/

    case 21 :
        newCde = new Commande(cde->getIdCde(),(char)137);

        if (cde->getNbArgs() != 1)
        {
            newCde->setError("",253,nameClt);
            break;
        }

        chan = channelByName(cde->getArg(1));

        if (chan == NULL)
            addChan(chan = new Channel(cde->getArg(1), nameClt));

        newCde->setDest(cde->getArg(1));

        if (chan->isBanned(nameClt))
        {
            newCde->setError("l'utilisateur est banni du channel",252,nameClt);
            break;
        }

        chan->addUser((searchClt(nameClt)).front());

        newCde->addArg(cde->getArg(1));
        newCde->addArg(nameClt);
        break;

        /*------/nick newNick-----*/

    case 22 :
        newCde = new Commande(cde->getIdCde(),(char)132);

        newCde->setDest(nameClt);

        if (cde->getNbArgs() != 1)
        {
            newCde->setError("",253,nameClt);
            break;
        }

        if (cde->getArg(1) == "")
        {
            newCde->setError("Nick invalide",250,nameClt);
            break;
        }

        if (changeNameClt(nameClt,cde->getArg(1)) == -1)
        {
            cde->setError("Nick déja pris",250,nameClt);
            break;
        }

        newCde->addArg(nameClt);
        newCde->addArg(cde->getArg(1));

        break;


        /*-----/leave channel----*/

    case 23 :
        newCde = new Commande(cde->getIdCde(),(char)133);

        if (cde->getNbArgs() != 1)
        {
            newCde->setError("",253,nameClt);
            break;
        }

        chan = channelByName(cde->getArg(1));

        if(chan == NULL)
        {
            newCde->setError("Le channel n'existe pas",254,nameClt);
            break;
        }

        newCde->setDest(cde->getArg(1));

        chan->kickClt(nameClt);

        newCde->addArg(cde->getArg(1));
        newCde->addArg(nameClt);

        if (chan->isEmpty())
            chan->~Channel();

        break;


        /*-----/unban channel motif-----*/

    case 24 :
        newCde = new Commande(cde->getIdCde(),(char)135);

        if (cde->getNbArgs() != 2)
        {
            newCde->setError("",253,nameClt);
            break;
        }

        chan = channelByName(cde->getArg(1));

        if(chan == NULL)
        {
            newCde->setError("Le channel n'existe pas",254,nameClt);
            return NULL;
        }

        newCde->setDest(cde->getArg(1));

        chan->supprBanned(cde->getArg(2));


        newCde->addArg(cde->getArg(1));
        newCde->addArg("-");
        newCde->addArg(cde->getArg(2));

        break;


        /*-----/banlist channel----*/

    case 25 :
        newCde = new Commande(cde->getIdCde(),(char)129);

        newCde->setDest(nameClt);

        if (cde->getNbArgs() != 1)
        {
            newCde->setError("",253,nameClt);
            break;
        }

        chan = channelByName(cde->getArg(1));

        if(chan == NULL)
        {
            newCde->setError("Le channel n'existe pas",254,nameClt);
            break;
        }

        cltSearch = chan->listBan();

        newCde->addArg("Server");

        for (list<Client*>::const_iterator it=cltSearch.begin() ; it!=cltSearch.end() ; ++it)
            newCde->addArg((*it)->getName());

        cltSearch.erase(cltSearch.begin(),cltSearch.end());
        break;

    }
    return newCde;
}


void Server::send(Commande *cde)
{
    Channel* chan;

    char trame[4096] ;

    cde->createMsg(trame) ;

    switch (cde->getCde()) {



    case 128 :
        chan = channelByName(cde->getDest());
        chan->broadcast(trame);
        break;

    case 129 :
        writeToClt(trame,cde->getDest());
        break;

    case 130 :

        break;

    case 131 :
        chan = channelByName(cde->getDest());
        chan->broadcast(trame);
        break;

    case 132 :
        writeToClt(trame,cde->getDest());
        break;

    case 133 :
        chan = channelByName(cde->getDest());
        chan->broadcast(trame);
        break;

    case 134 :
        chan = channelByName(cde->getDest());
        chan->broadcast(trame);
        break;

    case 135 :
        chan = channelByName(cde->getDest());
        chan->broadcast(trame);
        break;

    case 136 :
        broadcast(trame);
        break;

    case 137 :
        chan = channelByName(cde->getDest());
        chan->broadcast(trame);
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
    char cde, buf[4096];
    memset(buf,0,sizeof(buf));

    read(sock,&sizeTrame,sizeof(sizeTrame));
    read(sock,&idCde,sizeof(idCde));
    read(sock,&cde,sizeof(cde));

    int tbuf=sizeTrame-sizeof(idCde)-sizeof(cde);

    read(sock,buf,tbuf);

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

bool Server::isClt(const string nameClt) const
{
    for(list<Client*>::const_iterator it=clients.begin() ; it!=clients.end() ; it++)
        if ((*it)->getName() == nameClt)
            return true;
    return false;
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
    else
        cout<<"Erreur : "<<errno<<endl;

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

void Server::broadcast(const char *message)
{
    uint16_t B ;
    uint16_t *taille = &B ;
    memcpy(taille, message,sizeof(B)) ;

    for (list<Client*>::iterator i=clients.begin() ; i != clients.end() ; ++i)
    {
        write((*i)->getSock(), message,*taille+2);
    }
}
