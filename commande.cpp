#include <iostream>
#include "commande.h"

Commande::Commande(uint16_t idCde, char cde)
    :idCde(idCde), cde(cde)
{
}

void Commande::addArg(const string &arg)
{
    args.push_back(arg);
    ++nbArgs;
}

void Commande::affichArgs()
{
    for (list<string>::const_iterator it=args.begin() ; it != args.end() ; ++it)
        cout<<*it<<endl;
}

const char &Commande::getCde() const
{
    return cde;
}

const int &Commande::getNbArgs() const
{
    return nbArgs;
}

string Commande::getArg(const int num) const
{
    int i=1;

    for(list<string>::const_iterator it=args.begin(); it != args.end() ; ++it)
    {
        if(i == num)
            return (*it);
        ++i;
       }

    return "ERROR";
}

void Commande::setError(const string err)
{
    error = err;
}
