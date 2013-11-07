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

    Client(int sock);

    const int & getSock() const;

    const string & getName() const;
    void setName(const string name);
};

#endif // CLIENT_H
