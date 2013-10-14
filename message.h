#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>

using namespace std;

class Message
{
    string mess;
    string nameClt;

public:

    static int nbMess;

    Message(string messNew, string nameCltNew);

    string getMess();

    string getName();
};

#endif // MESSAGE_H
