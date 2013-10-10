/*
  Auteurs : Ducros Alix & Lefebvre De Ladonchamps Rémi
  Fichier : main.cpp
  Fonctionnement : Serveur IRC.
 */

#include "server.h"


/*Initialisation de la socket name*/

void init_sockaddr (struct sockaddr_in *name,
               const char *hostname,
               uint16_t port)
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



/*
  Argument : port : si absent 1025 par défaut.
 */

int main(int argc, char **argv)
{
    char buf[4096];
    unsigned port;
    int retval, tbuf;
    fd_set readfd;
    struct sockaddr_in sin, csin;
    int sock=socket(AF_INET,SOCK_STREAM,0);
    socklen_t taille=sizeof(csin);
    list<Client*> client;

    if(argc >= 2 )
            if ( sscanf(argv[1],"%u",&port) != 1  )
            {
                fprintf(stderr,"Numéro de port invalide\n");
                close(sock);
                exit(1);
            }

    if (argc < 2)
        {
             port = 1025 ;
        }

    cout << "Port ouvert : " << port << endl;

    if (sock+1 > Client::maxSock)
            Client::maxSock = sock+1;

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

    while(1)
        {

            addAllSockets(client, &readfd, sock);

            retval = select(Client::maxSock,&readfd, NULL, NULL, NULL);

            if (retval == -1)
                    {
                        perror("Select ");
                        closeAllSockets(client);
                        close(sock);
                        exit(1);
                    }

            if (FD_ISSET(sock, &readfd))
                    {
                        list<Client*>::iterator i;

                        client.push_front(new Client(accept(sock,(struct sockaddr *) &csin, &taille), "default"));
                        printf("Client connecté\n");
                        i = client.begin();
                        tbuf = sizeof(buf);
                        printf("Taille du buffer : %lu\n",sizeof(buf));
                        write((*i)->getSock(),&tbuf,sizeof(int));
                    }

            for(list<Client*>::iterator i=client.begin(); i != client.end() ; ++i)
                        if (FD_ISSET((*i)->getSock(), &readfd))
                        {
                            read((*i)->getSock(),buf,sizeof(buf));
                            cout << buf;
                            fflush(stdout);
                        }


        }

    closeAllSockets(client);
    client.erase(client.begin(),client.end());
    close(sock);

    return 0;
}

