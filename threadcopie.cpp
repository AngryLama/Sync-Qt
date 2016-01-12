#include "threadcopie.h"

ThreadCopie::ThreadCopie(QString _source,QString _destination,int _mode):source(_source),destination(_destination),mode(_mode)
{

}

void ThreadCopie::run()
{
    switch (mode) {
    case COPIE:
        if(QFile::exists(destination))
            QFile::remove(destination);
        QFile::copy(source,destination);
        break;
    case DEPLACE:
        QFile::rename(source,destination);
        break;
    default:
        break;
    }
}

