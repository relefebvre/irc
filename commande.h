#ifndef COMMANDE_H
#define COMMANDE_H

#include <stdint.h>
#include <vector>
#include <string>
#include <map>

using namespace std;

class Commande
{
    uint16_t idCde;
    unsigned char cde;
    vector<string> args;
    char errNum;
    string err;
    string dest;


public:
    static map<char, string> errors;

    Commande(uint16_t idCde, char cde);
    ~Commande();

    void setCde(char newCde);

    void addArg(const string & arg);

    const unsigned char & getCde() const;

    const uint16_t & getIdCde() const;

    int getNbArgs() const { return args.size(); }

    const string & getArg(const int num) const;

    void setError(const string err,const char errNum, const string nameClt);

    int createMsg(char *trame) const;

    static void initErrors();

    void setDest(const string destinataire);
    const string getDest() const;
};

#endif // COMMANDE_H
