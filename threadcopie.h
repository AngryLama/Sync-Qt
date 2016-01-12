#ifndef THREADCOPIE_H
#define THREADCOPIE_H

#include <QThread>
#include <QFile>

#define COPIE 0
#define DEPLACE 1

class ThreadCopie : public QThread
{
public:
    ThreadCopie(QString _source, QString _destination, int _mode=COPIE);
    void run();

private:
    QString source,destination;
    int mode;
};

#endif // THREADCOPIE_H
