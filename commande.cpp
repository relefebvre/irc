#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "commande.h"
#include <cassert>

Commande::Commande(uint16_t idCde, char cde)
    :idCde(idCde), cde(cde)
{
    errNum=0;
}

void Commande::addArg(const string &arg)
{
    args.push_back(arg);
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


string Commande::getArg(const int num) const
{
    int i=1;

    for(list<string>::const_iterator it=args.begin(); it != args.end() ; ++it)
    {
        if(i == num)
            return (*it);
        ++i;
       }
    assert(true);
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


Message* Commande::createMsg() const
{
    string mess;
    char *trame;
    uint16_t size;
    for(list<string>::const_iterator it=args.begin(); it != args.end() ; ++it)
        mess += (*it)+"\n";
    size = mess.length()+sizeof(idCde)+sizeof(cde);
    trame = new char[size+sizeof(size)];

    memcpy(trame,&size,sizeof(size));
    memcpy(trame+sizeof(size),&idCde,sizeof(idCde));
    memcpy(trame+sizeof(size)+sizeof(idCde),&cde,sizeof(cde));
    memcpy(trame+sizeof(size)+sizeof(idCde)+sizeof(cde),mess.c_str(),mess.length());

    return new Message(trame,size+sizeof(size));
}
