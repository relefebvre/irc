#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "commande.h"
#include <cassert>
#include <fstream>

map<char, string> Commande::errors;

Commande::Commande(uint16_t idCde, char cde)
    :idCde(idCde), cde(cde)
{
    errNum=0;
}

void Commande::setCde(char newCde)
{
    cde = newCde;
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
    return args[num-1];
}

void Commande::setError(const string err, const char errNum, const string nameClt)
{
    this->errNum = errNum;

    if (err == "")
        this->err = errors[errNum];
    else
        this->err = err;
    cde = 129;
    setDest(nameClt);
    addArg("Server");
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

    if (errNum != 0)
    {
        size = err.length()+3;
        mess = err;
    }

    memcpy(trame,&size,sizeof(size));
    memcpy(trame+2,&idCde,sizeof(idCde));
    if (errNum == 0)
        memcpy(trame+4,&cde,sizeof(cde));
    else
        memcpy(trame+4,&errNum,sizeof(errNum));
    memcpy(trame+5,mess.c_str(),strlen(mess.c_str()));
    return 0 ;
}

void Commande::initErrors()
{
    ifstream fic("errors.don",ios::in);

    if (fic)
    {
        int errNum;
        while(fic >> errNum)
        {
            string err;
            getline(fic,err);
            errors[errNum] = err;
        }
        fic.close();
    }
    else
        cout<<"Erreur d'ouverture de fichier"<<endl;

}

void Commande::setDest(const string destinataire)
{
    dest = destinataire;
}

const string Commande::getDest() const
{
    return dest;
}
