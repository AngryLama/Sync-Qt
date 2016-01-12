#ifndef THREADDIRITERATOR_H
#define THREADDIRITERATOR_H

#include <QThread>
#include <QDirIterator>

typedef enum{
    DOSSIER,
    FICHIER
}ModeRecherche;

class ThreadDirIterator : public QThread
{
    Q_OBJECT
public:
    ThreadDirIterator(QString source,QStringList filtre,ModeRecherche _type,bool sousDossiers);
    ~ThreadDirIterator();
    void run();

signals:
    void finRecherche(QStringList);

private:
    QDirIterator *iterator;
    QStringList retour;
    ModeRecherche type;
};

#endif // THREADDIRITERATOR_H
