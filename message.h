#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>

using namespace std;

class Message
{
    char* mess;
    int size;

public:

    Message(char *mess, int size);

    const char* getMess() const;

    int getSize() const;

};

#endif // MESSAGE_H
