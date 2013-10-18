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
    string buf;
    int tbuf, retval;
    int sock ;
    uint port = 1025 ;
    fd_set readfd;
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

    write(sock,nameClt,strlen(nameClt));

    uint16_t idCde = 99, sizeT;
    char c='1';
    char tot[]="test\ntast1\ntzst2\ntrst2\n";

    sizeT = sizeof(tot)+5;

    cout<<sizeT<<idCde<<c<<tot<<endl;

    char tot2[sizeT];
    sprintf(tot2,"%u%u%c%s",sizeT,idCde,c,tot);
    cout<<tot2<<endl;
    write(sock,tot2,sizeof(tot2)-1);
    exit(1);

    while(1)
    {

        FD_ZERO(&readfd);
        FD_SET(sock, &readfd);
        FD_SET(0, &readfd);


        retval = select(sock+1, &readfd, NULL, NULL, NULL);

        if (retval == -1)
        {
            perror("Select ");
            close(sock);
            exit(1);
        }

        if (FD_ISSET(sock, &readfd))
        {
            char buf[tbuf];
            int nbLu;

            nbLu = read(sock,buf,tbuf);
            buf[nbLu]='\0';

            cout << "Message reçu : " << buf << endl;
        }

        if (FD_ISSET(0, &readfd))
        {
            /*char buf[tbuf];
            int nbLu;
            nbLu = read(0,buf,tbuf);
            if(buf[strlen(buf)-1]=='\n')
            {
                buf[strlen(buf)-1]='\0';
                ++nbLu;
            }
            write(sock,buf,nbLu);*/



        }
    }



    close(sock);

    return 0;
}





