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
    return args[num];
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


int Commande::createMsg(char *trame) const
{
    string mess;
    uint16_t size;
    for(unsigned int i=0 ; i<args.size() ; ++i)
        mess += args[i]+"\n";
    size = mess.length()+3;
    cout<<"taille : "<<size<<endl;
#warning Ici sans doute erreur de generation de la trame
    memcpy(trame,&size,sizeof(size));
    memcpy(trame+2,&idCde,sizeof(idCde));
    memcpy(trame+4,&cde,sizeof(cde));
    memcpy(trame+5,mess.c_str(),strlen(mess.c_str()));
    return 0 ;
}
