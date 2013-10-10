#include "server.h"


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
    unsigned port;
    int sock=socket(AF_INET,SOCK_STREAM,0);

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
    return 0;
}

