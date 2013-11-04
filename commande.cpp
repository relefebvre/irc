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
    assert(false);
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
    char trame[4096]="0";
    uint16_t size;
    for(list<string>::const_iterator it=args.begin(); it != args.end() ; ++it)
        mess += (*it)+"\n";
    size = mess.length()+3;
    cout<<"taille : "<<size<<endl;
#warning Ici sans doute erreur de generation de la trame
    //sprintf(protocol,"%u%d%x",size,idCde,errNum);
    memcpy(trame,&size,sizeof(size));
    memcpy(trame+sizeof(size),&idCde,sizeof(idCde));
    memcpy(trame+sizeof(size)+sizeof(idCde),&cde,sizeof(cde));
    memcpy(trame+sizeof(size)+sizeof(idCde)+sizeof(cde),mess.c_str(),strlen(mess.c_str()));
    printf("Message : %s",trame);
    mess=trame;
    return mess;
}
