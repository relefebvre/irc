#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <netdb.h>
#include <fcntl.h>
#include <iostream>

using namespace std;


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

int main(int argc, char **argv)
{
    int tbuf;
    int sock ;
    uint port = 1025 ;
    struct sockaddr_in sin;
    char IPdefault[10] = "127.0.0.1" ;
    char nameClt[40] = "Anonyme";
    int arg = 0, ins = 0;

    for (arg = 1 ; arg < (argc-1); arg+=2) {
        if (strcmp(argv[arg], "-p") == 0)
                ins = 1 ;
        if (strcmp(argv[arg], "-ip") == 0)
                ins = 2 ;
        if (strcmp(argv[arg], "-id") == 0)
                ins = 3 ;

        switch (ins) {
            case 1:
                if(sscanf(argv[arg+1],"%u",&port) != 1 )
                    {
                     fprintf(stderr,"Numéro de port invalide\n");
                     exit(1);
                     }

                 break;
            case 2:
                if (sscanf(argv[arg+1],"%s",IPdefault) != 1 )
                {
                    fprintf(stderr,"IP non valide\n");
                    exit(1);
                }

                 break;
            case 3:
                if (sscanf(argv[arg+1],"%s",nameClt) != 1 )
                {
                    fprintf(stderr,"Nom de client invalide\n");
                    exit(1);
                }

                 break;
            default:
                fprintf(stderr,"Option invalide\n");
                exit(1);
                 break;
        }
        ins = 0 ;
    }

    init_sockaddr(&sin,IPdefault,port);




    sock=socket(AF_INET,SOCK_STREAM,0);

    if (connect(sock, (struct sockaddr *) &sin, sizeof(sin)) != 0)
    {
        perror("Connexion échouée ") ;
        close(sock) ;
        exit(1) ;
    }

    if (read(sock,&tbuf,sizeof(int)) == -1)
    {
        perror("Lecture ");
        close(sock);
        exit(1);
    }

    printf("Taille du buffer : %d\n",tbuf);

    cout<<"NameClt : "<<nameClt<<endl;

    int size=strlen(nameClt);
    write(sock,&size,sizeof(size));
    write(sock,nameClt,strlen(nameClt));



    uint16_t idCde = 99, sizeT;
    char c=(char)1;
    char tot[]="toto\ntest message pour toto\n";

    sizeT = sizeof(tot)+sizeof(idCde)+sizeof(c);

    if(sizeT < 10)
    {
        cout<<"sizeT < 10"<<endl;
        exit(1);
    }



    char tot2[sizeT+sizeof(sizeT)];
    sprintf(tot2,"%u%u%c%s",sizeT,idCde,c,tot);
    cout<<tot2<<endl;
    write(sock,tot2,sizeof(tot2)-1);

    char buf[tbuf];

    while(read(sock,buf,tbuf) > 0)
    {

            cout << "Message reçu : " << buf;


    }



    close(sock);

    return 0;
}





