#ifndef CLIENT_H
#define CLIENT_H

#include <string>

using namespace std;

class Client
{
    int sock;
    string name;

public:

    static int maxSock;

    Client(int newSock, string newName);

    int getSock();
};

#endif // CLIENT_H
