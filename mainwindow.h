#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QDirIterator>
#include <QDateTime>
#include <QSql>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include "threadcopie.h"
#include "threaddiriterator.h"

typedef enum{
    AUCUN,
    CONTRIBUTION,
    SYNCRONISATION,
    ECHO
}Mode;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void verifListes();
    void changerMode(Mode val);
    void changerMode(bool etat);
    void analyseContribution();
    void analyseSyncronisation();
    void analyseEcho();
    void resetListes();

private slots:
    void on_pushButton_Source_clicked();

    void on_pushButton_Destination_clicked();

    void on_progressBar_valueChanged(int value);

    void on_pushButton_Lancer_clicked();

    void on_finCopieSource();

    void on_finCopieDestination();

    void on_finFichiersSource(QStringList liste);

    void on_finDossiersSource(QStringList liste);

    void on_finFichiersDestination(QStringList liste);

    void on_finDossiersDestination(QStringList liste);

    void on_finRecherche();

    void on_pushButton_nouveauProfil_clicked();

    void on_verifChamps();

    void on_pushButton_supprimerProfil_clicked();

    void on_listWidget_profils_currentTextChanged(const QString &currentText);

    void on_pushButton_annulerProfil_clicked();

    void on_lineEdit_nomProfil_textChanged(const QString &arg1);

    void analyser();

private:
    Ui::MainWindow *ui;
    QSqlDatabase db;
    ThreadCopie *copie;
    ThreadDirIterator *recherche[4];

    Mode mode;

    //Dossiers de base
    QString source,destination;

    //Outils de liste
    QStringList filtre;

    //Listes générées
    QStringList fichiersSource,fichiersDestination,repertoiresSource,repertoiresDestination;

    //Listes des transferts
    QStringList fichiersVersDestination,repertoiresVersDestination,fichiersVersSource,repertoiresVersSource;

    //Listes des suppressions
    QStringList fichiersSupprDestination,repertoiresSupprDestination;
};

#endif // MAINWINDOW_H
