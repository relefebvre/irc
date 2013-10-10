#include "client.h"

int Client::maxSock = 0;

Client::Client(int newSock)
{
    this->sock = newSock;
    if (newSock+1 > Client::maxSock)
        Client::maxSock = newSock+1;
}
