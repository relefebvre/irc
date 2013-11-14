#include "channel.h"
#include "client.h"
#include <string.h>
#include <unistd.h>
#include <regex.h>
#include <stdint.h>
#include <iostream>


/*
 *      Constructeur de Channel
 *
 *
 */


Channel::Channel(string chanName,string op)
    : chanName(chanName), op(op)
{
    topic="Default Topic";
}

/*
 *      Destructeur de Channel
 *          - Supprime les Clients utilisant le Channel (C'est peut-être bancal)
 *          - Supprime les Clients bannis du Channel (Ca aussi)
 *          - Vide les listes
 *
 */


Channel::~Channel()
{
    for (list<Client*>::iterator it=users.begin() ; it!=users.end() ; ++it)
        delete *it;

    for (list<Client*>::iterator it=banned.begin() ; it!=banned.end() ; ++it)
        delete *it;

    users.erase(users.begin(), users.end());
    banned.erase(banned.begin(), banned.end());
}

/*
 *      IsOp
 *          - Vérifie si le nom du Client passé en paramètre correspond à celui de l'op du Channel
 *
 *
 */

bool Channel::isOp(const string nameClt) const
{
    return nameClt == op;
}

/*
 *      GetChanName
 *          - retourne le nom du Channel
 *
 */

const string &Channel::getChanName() const
{
    return chanName;
}

/*
 *      GetTopic
 *          - Retourne le topic du Channel
 *
 */


const string &Channel::getTopic() const
{
    return topic;
}

/*
 *      SetTopic
 *          - Définit la chaine passée en paramètre comme topic du Channel
 *
 */

void Channel::setTopic(const string topic)
{
    this->topic = topic;
}

/*
 *      Broadcast
 *          - Transmet la chaine message à toute la liste d'utilisateur du Channel
 *
 */

void Channel::broadcast(const char *message)
{
    uint16_t B ;
    uint16_t *taille = &B ;
    memcpy(taille, message,sizeof(B)) ;

    for (list<Client*>::iterator i=users.begin() ; i != users.end() ; ++i)
    {
        write((*i)->getSock(), message,*taille+2);
    }
}

/*
 *      AddUser
 *          - Ajoute un Client à la liste de Client du Channel
 *
 */


void Channel::addUser(Client* newClt)
{
    users.push_back(newClt);
}

/*
 *      SearchClt
 *          - Retourne la liste de Clients du Channel
 *
 */


list<Client*> Channel::searchClt() const
{
    return users;
}

/*
 *      KickClt
 *          - Supprime un Client de la liste d'utilisateurs
 *          - Renvoie une liste de Client avec le Client kické dedans
 *
 */


list<Client*> Channel::kickClt(const string motif)
{
    list<Client*> clt;
    regex_t expr;

    if ( (regcomp(&expr, motif.c_str(),REG_EXTENDED)) == 0)
    {

        for (list<Client*>::iterator i=users.begin() ; i != users.end() ; ++i)
        {
            if ((regexec(&expr,(*i)->getName().c_str(),0,NULL,0)) == 0)
            {
                clt.push_back(*i);
                i = users.erase(i);
            }
        }
    }
    return clt;
}

/*
 *      AddBanned
 *          - Ajoute un Client à la liste de bannis
 *          - Ejecte le Client du Channel
 *          - renvoie 1 si l'opération a été effectuée, 0 sinon.
 *
 */

int Channel::addBanned(const string motif)
{
    int nb=0;
    regex_t expr;

    if ( (regcomp(&expr, motif.c_str(),REG_EXTENDED)) == 0)
    {
        for (list<Client*>::const_iterator i=users.begin() ; i != users.end() ; ++i)
            if ((regexec(&expr,(*i)->getName().c_str(),0,NULL,0)) == 0)
            {
                banned.push_back(*i);
                ++nb;
                kickClt((*i)->getName());
            }
    }
    return nb;
}

/*
 *      SupprBanned
 *          - Supprime un Client de la liste de bannis
 *          - Renvoie 1 si l'opération a été effectuée, 0 sinon
 *
 *
 */

int Channel::supprBanned(const string motif)
{
    int nb=0;
    regex_t expr;

    if ( (regcomp(&expr, motif.c_str(),REG_EXTENDED)) == 0)
    {
        for (list<Client*>::iterator i=users.begin() ; i != users.end() ; ++i)
            if ((regexec(&expr,(*i)->getName().c_str(),0,NULL,0)) == 0)
            {
                i = banned.erase(i);
                ++nb;
            }
    }
    return nb;
}

/*
 *      SetOp
 *          - Ajoute un Client de la liste d'utilisateur en tant que op (remplaçant l'ancien)
 *
 */

int Channel::setOp(const string nameClt)
{
    for (list<Client*>::const_iterator i=users.begin() ; i != users.end() ; ++i)
        if ((*i)->getName() == nameClt)
        {
            op=nameClt;
            return 0;
        }
    return -1;
}

/*
 *      SupprOp
 *          - Si le nom passé en paramètre correspond à un Client du Channel, l'op est mis à nul
 *
 */

int Channel::supprOp(const string nameClt)
{
    for (list<Client*>::const_iterator i=users.begin() ; i != users.end() ; ++i)
        if ((*i)->getName() == nameClt)
        {
            op="";
            return 0;
        }
    return -1;
}

/*
 *      IsBanned
 *          - Renvoie true si le nom passé en paramètre correspond à un un Client de la liste de bannis
 *
 */


bool Channel::isBanned(const string nameClt) const
{
    for (list<Client*>::const_iterator i=banned.begin() ; i != banned.end() ; ++i)
        if ((*i)->getName() == nameClt)
        {
            return true;
        }
    return false;
}

/*
 *      ListBan
 *          - Renvoie la liste de bannis
 *
 */

list<Client*> Channel::listBan() const
{
    return banned;
}

/*
 *      IsEmpty
 *          - renvoie true si la liste de Client est vide
 *
 */

bool Channel::isEmpty() const
{
    return users.empty();
}
