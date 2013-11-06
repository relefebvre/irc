#ifndef COMMANDE_H
#define COMMANDE_H

#include <unistd.h>
#include <stdint.h>
#include <vector>
#include <string>

using namespace std;

class Commande
{
    uint16_t idCde;
    unsigned char cde;
    vector<string> args;
    int errNum;
    string err;

public:
    Commande(uint16_t idCde, char cde);

    void addArg(const string & arg);

    const unsigned char &getCde() const;

    const uint16_t & getIdCde() const;

    int getNbArgs() const { return args.size(); }

    string getArg(const int num) const;

    void setError(const string err,const int errNum);
    const string & getError() const;

    int createMsg(char *trame) const;
};

#endif // COMMANDE_H
