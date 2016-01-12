#include "threaddiriterator.h"

ThreadDirIterator::ThreadDirIterator(QString source, QStringList filtre, ModeRecherche _type, bool sousDossiers):type(_type)
{
    switch (_type) {
    case DOSSIER:
        if(sousDossiers)
            iterator=new QDirIterator(source,filtre,QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        else iterator=new QDirIterator(source,filtre,QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
        break;
    case FICHIER:
        if(sousDossiers)
            iterator=new QDirIterator(source,filtre,QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        else iterator=new QDirIterator(source,filtre,QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
        break;
    default:
        break;
    }

}

ThreadDirIterator::~ThreadDirIterator()
{
    delete iterator;
}

void ThreadDirIterator::run()
{
    while(iterator->hasNext())
    {
        retour << iterator->next();
    }

    emit finRecherche(retour);
}

