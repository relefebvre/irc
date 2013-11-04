#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "commande.h"

Commande::Commande(uint16_t idCde, char cde)
    :idCde(idCde), cde(cde)
{
    nbArgs=0;
    errNum=0;
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

const unsigned char &Commande::getCde() const
{
    return cde;
}

 const uint16_t & Commande::getIdCde() const
 {
     return idCde;
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

void Commande::setError(const string err, const int errNum)
{
    this->errNum = errNum;

    if (err == "");
        //Erreur par défeut //Map<message erreur, code erreur>////
    else
        this->err = err;
}

const string &Commande::getError() const
{
    return err;
}


const string Commande::createMsg() const
{
    string mess;
    uint16_t size;
    char protocol[5];

    for (int i=1 ; i<=nbArgs ; ++i)
        mess += getArg(i)+"\n";
    size = mess.length()+3;
    sprintf(protocol,"%d%d%x",size,idCde,cde);
    mess = protocol+mess+"\n";
    return mess;
}
