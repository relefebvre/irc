#ifndef COMMANDE_H
#define COMMANDE_H

#include <unistd.h>
#include <stdint.h>
#include <list>
#include <string>

using namespace std;

class Commande
{
    uint16_t idCde;
    char cde;
    list<string> args;
    int nbArgs;
    string error;

public:
    Commande(uint16_t idCde, char cde);

    void addArg(const string & arg);

    void affichArgs();

    const char & getCde() const;

    const int & getNbArgs() const;

    string getArg(const int num) const;

    void setError(const string err);
};

#endif // COMMANDE_H
