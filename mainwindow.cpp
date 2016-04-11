#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setFixedSize(710,355);

    ui->label_fichierEnCours->hide();
    ui->label_Wait->hide();

    //Initialisation du tableWidget_Resume
    ui->tableWidget_Resume->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableWidget_Resume->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableWidget_Resume->setItem(0,0,new QTableWidgetItem(QString::number(0)));
    ui->tableWidget_Resume->setItem(0,1,new QTableWidgetItem(QString::number(0)));
    ui->tableWidget_Resume->setItem(0,2,new QTableWidgetItem(QString::number(0)));
    ui->tableWidget_Resume->setItem(1,0,new QTableWidgetItem(QString::number(0)));
    ui->tableWidget_Resume->setItem(1,1,new QTableWidgetItem(QString::number(0)));
    ui->tableWidget_Resume->setItem(1,2,new QTableWidgetItem(QString::number(0)));
    ui->tableWidget_Resume->item(0,0)->setTextAlignment(Qt::AlignCenter);
    ui->tableWidget_Resume->item(0,1)->setTextAlignment(Qt::AlignCenter);
    ui->tableWidget_Resume->item(0,2)->setTextAlignment(Qt::AlignCenter);
    ui->tableWidget_Resume->item(1,0)->setTextAlignment(Qt::AlignCenter);
    ui->tableWidget_Resume->item(1,1)->setTextAlignment(Qt::AlignCenter);
    ui->tableWidget_Resume->item(1,2)->setTextAlignment(Qt::AlignCenter);
    copie=NULL;
    for(int x=0;x<4;x++)
        recherche[x]=NULL;

    //Ouverture de la base de données
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("profils.sqlite");
    ui->groupBox_profils->setEnabled(db.open());

    //Remplissage de la liste des profils
    QSqlQuery query("SELECT Nom FROM Profils;");
    if(query.exec())
    {
        while(query.next())
        {
            ui->listWidget_profils->addItem(query.value(0).toString());
        }
    }
    else ui->groupBox_profils->setEnabled(false);

    ui->listWidget_profils->setFixedHeight(ui->listWidget_profils->height()+20);
    ui->lineEdit_nomProfil->hide();
    ui->pushButton_annulerProfil->hide();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::analyser()
{
    QPushButton *temp=(QPushButton*)sender();
    if(temp==ui->pushButton_Contri)
        changerMode(CONTRIBUTION);
    if(temp==ui->pushButton_Sync)
        changerMode(SYNCRONISATION);
    if(temp==ui->pushButton_Echo)
        changerMode(ECHO);

    if(mode != AUCUN)
    {
        //Reset listes
        resetListes();
        fichiersSource.clear();
        fichiersDestination.clear();
        dossiersSource.clear();
        dossiersDestination.clear();

        changerMode(false);
        ui->label_Wait->setText("Veuillez patienter...");
        ui->label_Wait->show();
        ui->progressBar->setMaximum(0);

        recherche[0]=new ThreadDirIterator(source,filtre,FICHIER,true);
        connect(recherche[0],SIGNAL(finRecherche(QStringList)),this,SLOT(on_finFichiersSource(QStringList)));

        recherche[1]=new ThreadDirIterator(source,filtre,DOSSIER,true);
        connect(recherche[1],SIGNAL(finRecherche(QStringList)),this,SLOT(on_finDossiersSource(QStringList)));

        recherche[2]=new ThreadDirIterator(destination,filtre,FICHIER,true);
        connect(recherche[2],SIGNAL(finRecherche(QStringList)),this,SLOT(on_finFichiersDestination(QStringList)));

        recherche[3]=new ThreadDirIterator(destination,filtre,DOSSIER,true);
        connect(recherche[3],SIGNAL(finRecherche(QStringList)),this,SLOT(on_finDossiersDestination(QStringList)));

        for(int x=0;x<4;x++)
        {
            connect(recherche[x],SIGNAL(finished()),this,SLOT(on_finRecherche()));
            recherche[x]->start();
        }
    }
}

void MainWindow::changerMode(Mode val)
{
    mode=val;
    changerMode(true);

    switch (val) {
    case ECHO:
        ui->pushButton_Echo->setEnabled(false);
        break;
    case CONTRIBUTION:
        ui->pushButton_Contri->setEnabled(false);
        break;
    case SYNCRONISATION:
        ui->pushButton_Sync->setEnabled(false);
        break;
    default:
        break;
    }
}

void MainWindow::changerMode(bool etat)
{
    ui->pushButton_Contri->setEnabled(etat);
    ui->pushButton_Sync->setEnabled(etat);
    ui->pushButton_Echo->setEnabled(etat);
}

void MainWindow::analyserContribution()
{
    resetListes();
    QString temp;
    int tempI=0;
    for(int x=0;x<fichiersSource.length();x++){
        temp = destination+fichiersSource[x];
        QFile fichier(temp);
        if(!fichier.open(QFile::ReadOnly)){
            tempI++;
            ui->tableWidget_Resume->item(1,1)->setText(QString::number(tempI));
            fichiersVersDestination<<fichiersSource[x];
        }else{
            QFileInfo dateSource(source+fichiersSource[x]);
            QFileInfo dateDest(fichier);
            if(dateDest.lastModified()!=dateSource.lastModified()){
                tempI++;
                ui->tableWidget_Resume->item(1,1)->setText(QString::number(tempI));
                fichiersVersDestination<<fichiersSource[x];
            }
            fichier.close();
        }
    }

    tempI=0;
    for(int x=0;x<dossiersSource.length();x++){
        temp = destination+dossiersSource[x];
        QDir dossier(temp);
        if(!dossier.isReadable()){
            tempI++;
            ui->tableWidget_Resume->item(0,1)->setText(QString::number(tempI));
            dossiersVersDestination<<dossiersSource[x];
        }
    }

    tempI=0;
    for(int x=0;x<fichiersDestination.length();x++){
        temp = source+fichiersDestination[x];
        QFile fichier(temp);
        if(!fichier.open(QFile::ReadOnly)){
            tempI++;
            ui->tableWidget_Resume->item(1,0)->setText(QString::number(tempI));
            fichiersVersSource<<fichiersDestination[x];
        }else{
            QFileInfo dateSource(destination+fichiersDestination[x]);
            QFileInfo dateDest(fichier);
            if(dateDest.lastModified()!=dateSource.lastModified()){
                tempI++;
                ui->tableWidget_Resume->item(1,0)->setText(QString::number(tempI));
                fichiersVersSource<<fichiersDestination[x];
            }
            fichier.close();
        }
    }

    tempI=0;
    for(int x=0;x<dossiersDestination.length();x++){
        temp = source+dossiersDestination[x];
        QDir dossier(temp);
        if(!dossier.isReadable()){
            tempI++;
            ui->tableWidget_Resume->item(0,0)->setText(QString::number(tempI));
            dossiersVersSource<<dossiersDestination[x];
        }
    }
}

void MainWindow::analyserSyncronisation()
{
    resetListes();
    QString temp;
    int tempI=0;
    for(int x=0;x<fichiersSource.length();x++){
        temp = destination+fichiersSource[x];
        QFile fichier(temp);
        if(!fichier.open(QFile::ReadOnly)){
            tempI++;
            ui->tableWidget_Resume->item(1,1)->setText(QString::number(tempI));
            fichiersVersDestination<<fichiersSource[x];
        }else{
            QFileInfo dateSource(source+fichiersSource[x]);
            QFileInfo dateDest(fichier);
            if(dateDest.lastModified()!=dateSource.lastModified()){
                tempI++;
                ui->tableWidget_Resume->item(1,1)->setText(QString::number(tempI));
                fichiersVersDestination<<fichiersSource[x];
            }
            fichier.close();
        }
    }

    tempI=0;
    for(int x=0;x<dossiersSource.length();x++){
        temp = destination+dossiersSource[x];
        QDir dossier(temp);
        if(!dossier.isReadable()){
            tempI++;
            ui->tableWidget_Resume->item(0,1)->setText(QString::number(tempI));
            dossiersVersDestination<<dossiersSource[x];
        }
    }

    tempI=0;
    for(int x=0;x<fichiersDestination.length();x++){
        bool verif=false;
        for(int y=0;y<fichiersSource.length();y++){
            if(fichiersDestination[x]==fichiersSource[y])
                verif=true;
        }
        if(!verif){
            tempI++;
            ui->tableWidget_Resume->item(1,2)->setText(QString::number(tempI));
            fichiersSupprDestination<<fichiersDestination[x];
        }
    }

    tempI=0;
    for(int x=0;x<dossiersDestination.length();x++){
        bool verif=false;
        for(int y=0;y<dossiersSource.length();y++){
            if(dossiersDestination[x]==dossiersSource[y])
                verif=true;
        }
        if(!verif){
            tempI++;
            ui->tableWidget_Resume->item(0,2)->setText(QString::number(tempI));
            dossiersSupprDestination<<dossiersDestination[x];
        }
    }
}

void MainWindow::analyserEcho()
{
    resetListes();
    QString temp;

    //Analyse des fichiers à déplacer
    int tempI=0;
    for(int x=0;x<fichiersSource.length();x++){
        temp = destination+fichiersSource[x];
        QFile fichier(temp);
        if(!fichier.open(QFile::ReadOnly)){
            tempI++;
            ui->tableWidget_Resume->item(1,1)->setText(QString::number(tempI));
            fichiersVersDestination<<fichiersSource[x];
        }else{
            QFileInfo dateSource(source+fichiersSource[x]);
            QFileInfo dateDest(fichier);
            if(dateDest.lastModified()!=dateSource.lastModified()){
                tempI++;
                ui->tableWidget_Resume->item(1,1)->setText(QString::number(tempI));
                fichiersVersDestination<<fichiersSource[x];
            }
            fichier.close();
        }
    }

    //Analyse des fichiers à supprimer
    tempI=0;
    for(int x=0;x<fichiersDestination.length();x++){
        if(fichiersSource.count(fichiersDestination[x])==0){
            tempI++;
            ui->tableWidget_Resume->item(1,2)->setText(QString::number(tempI));
            fichiersSupprDestination<<fichiersDestination[x];
        }
    }

    //Analyse des dossiers à supprimer
    tempI=0;
    for(int x=0;x<dossiersDestination.length();x++){
        if(dossiersSource.count(dossiersDestination[x])==0){
            tempI++;
            ui->tableWidget_Resume->item(0,2)->setText(QString::number(tempI));
            dossiersSupprDestination<<dossiersDestination[x];
        }
    }

    //Analyse des répertoires à créer
    tempI=0;
    for(int x=0;x<dossiersSource.length();x++){
        temp = destination+dossiersSource[x];
        QDir dossier(temp);
        if(!dossier.isReadable()){
            tempI++;
            ui->tableWidget_Resume->item(0,1)->setText(QString::number(tempI));
            dossiersVersDestination<<dossiersSource[x];
        }
    }
}

void MainWindow::resetListes()
{
    //Reset listes
    fichiersVersDestination.clear();
    dossiersVersDestination.clear();
    fichiersVersSource.clear();
    dossiersVersSource.clear();
    fichiersSupprDestination.clear();
    dossiersSupprDestination.clear();

    //Remise à zéro tableau
    ui->tableWidget_Resume->item(0,0)->setText(QString::number(0));
    ui->tableWidget_Resume->item(0,1)->setText(QString::number(0));
    ui->tableWidget_Resume->item(0,2)->setText(QString::number(0));
    ui->tableWidget_Resume->item(1,0)->setText(QString::number(0));
    ui->tableWidget_Resume->item(1,1)->setText(QString::number(0));
    ui->tableWidget_Resume->item(1,2)->setText(QString::number(0));
}

void MainWindow::on_pushButton_Source_clicked()
{
    ui->lineEdit_Source->setText(QFileDialog::getExistingDirectory(this, tr("Choisir le dossier source"),
                                                                   "/home",
                                                                   QFileDialog::ShowDirsOnly
                                                                   | QFileDialog::DontResolveSymlinks));
}

void MainWindow::on_pushButton_Destination_clicked()
{
    ui->lineEdit_Destination->setText(QFileDialog::getExistingDirectory(this, tr("Choisir le dossier source"),
                                                                        "/home",
                                                                        QFileDialog::ShowDirsOnly
                                                                        | QFileDialog::DontResolveSymlinks));
}

void MainWindow::on_progressBar_valueChanged(int value)
{
    if(value==0)
        ui->label_Wait->hide();
    else ui->label_Wait->show();
    if(value==ui->progressBar->maximum())
        ui->label_Wait->setText("Terminé avec succès");
    else ui->label_Wait->setText("Veuillez patienter...");
}

void MainWindow::on_pushButton_Lancer_clicked()
{
    ui->label_fichierEnCours->show();
    ui->label_Wait->show();
    ui->pushButton_Lancer->setEnabled(false);
    ui->pushButton_Source->setEnabled(false);
    ui->pushButton_Destination->setEnabled(false);
    changerMode(false);
    ui->pushButton_Quitter->setEnabled(false);
    ui->progressBar->setMaximum(fichiersSupprDestination.length()+dossiersSupprDestination.length()+dossiersVersSource.length()+dossiersVersDestination.length()+fichiersVersSource.length()+fichiersVersDestination.length());
    QDir doss;

    //Suppression fichiers destination
    if(!fichiersSupprDestination.isEmpty()){
        for(int x=0;x<fichiersSupprDestination.length();x++){
            ui->progressBar->setValue(ui->progressBar->value()+1);
            doss.remove(destination+fichiersSupprDestination[x]);
        }
    }

    //Suppression dossiers destination
    if(!dossiersSupprDestination.isEmpty()){
        for(int x=0;x<dossiersSupprDestination.length();x++){
            ui->progressBar->setValue(ui->progressBar->value()+1);
            doss.rmdir(destination+dossiersSupprDestination[x]);
        }
    }

    //Création dossiers vers source
    if(!dossiersVersSource.isEmpty()){
        for(int x=0;x<dossiersVersSource.length();x++){
            ui->progressBar->setValue(ui->progressBar->value()+1);
            doss.mkdir(source+dossiersVersSource[x]);
        }
    }

    //Création dossiers vers destination
    if(!dossiersVersDestination.isEmpty()){
        for(int x=0;x<dossiersVersDestination.length();x++){
            ui->progressBar->setValue(ui->progressBar->value()+1);
            doss.mkdir(destination+dossiersVersDestination[x]);
        }
    }

    on_finCopieSource();
}

void MainWindow::on_finCopieSource()
{
    if(copie!=NULL)
        delete copie;
    if(!fichiersVersSource.isEmpty()){
        ui->progressBar->setValue(ui->progressBar->value()+1);
        ui->label_fichierEnCours->setText(source+fichiersVersSource.first());
        copie=new ThreadCopie(destination+fichiersVersSource.first(),source+fichiersVersSource.first());
        connect(copie,SIGNAL(finished()),this,SLOT(on_finCopieSource()));
        copie->start();
        fichiersVersSource.removeFirst();
    }else{
        on_finCopieDestination();
    }
}

void MainWindow::on_finCopieDestination()
{
    if(copie!=NULL)
        delete copie;
    if(!fichiersVersDestination.isEmpty()){
        ui->progressBar->setValue(ui->progressBar->value()+1);
        ui->label_fichierEnCours->setText(destination+fichiersVersDestination.first());
        copie=new ThreadCopie(source+fichiersVersDestination.first(),destination+fichiersVersDestination.first());
        connect(copie,SIGNAL(finished()),this,SLOT(on_finCopieDestination()));
        copie->start();
        fichiersVersDestination.removeFirst();
    }else{
        ui->pushButton_Source->setEnabled(true);
        ui->pushButton_Destination->setEnabled(true);
        changerMode(mode);
        ui->pushButton_Quitter->setEnabled(true);
        ui->label_fichierEnCours->hide();
        resetListes();
    }
}

void MainWindow::on_finFichiersSource(QStringList liste)
{
    fichiersSource=liste;
    delete recherche[0];
    recherche[0]=NULL;
}

void MainWindow::on_finDossiersSource(QStringList liste)
{
    dossiersSource=liste;
    delete recherche[1];
    recherche[1]=NULL;
}

void MainWindow::on_finFichiersDestination(QStringList liste)
{
    fichiersDestination=liste;
    delete recherche[2];
    recherche[2]=NULL;
}

void MainWindow::on_finDossiersDestination(QStringList liste)
{
    dossiersDestination=liste;
    delete recherche[3];
    recherche[3]=NULL;
}

void MainWindow::on_finRecherche()
{
    if(recherche[0]==NULL && recherche[1]==NULL && recherche[2]==NULL && recherche[3]==NULL)
    {
        for(int x=0;x<fichiersSource.length();x++)
            fichiersSource[x].remove(source);
        for(int x=0;x<dossiersSource.length();x++)
            dossiersSource[x].remove(source);
        for(int x=0;x<fichiersDestination.length();x++)
            fichiersDestination[x].remove(destination);
        for(int x=0;x<dossiersDestination.length();x++)
            dossiersDestination[x].remove(destination);

        ui->progressBar->setMaximum(100);
        ui->progressBar->setValue(0);
        ui->label_Wait->hide();

        switch (mode) {
        case CONTRIBUTION:
            analyserContribution();
            break;
        case SYNCRONISATION:
            analyserSyncronisation();
            break;
        case ECHO:
            analyserEcho();
            break;
        default:
            break;
        }
        ui->pushButton_Lancer->setEnabled(!(!fichiersVersDestination.length() && !dossiersVersDestination.length() && !fichiersVersSource.length() && !dossiersVersSource.length() && !fichiersSupprDestination.length() && !dossiersSupprDestination.length()));
        changerMode(mode);
    }
}

void MainWindow::on_pushButton_nouveauProfil_clicked()
{
    if(!ui->lineEdit_nomProfil->isVisible()){
        ui->listWidget_profils->setFixedHeight(ui->listWidget_profils->height()-20);
        ui->lineEdit_nomProfil->show();
        ui->pushButton_annulerProfil->show();
        ui->pushButton_nouveauProfil->setEnabled(false);
        ui->pushButton_nouveauProfil->setIcon(QIcon(":/Icon/Check.png"));
    }else{
        QSqlQuery query;
        query.prepare("INSERT INTO Profils (Nom, Source, Destination) VALUES (:v1,:v2,:v3);");
        query.bindValue(":v1",ui->lineEdit_nomProfil->text());
        query.bindValue(":v2",ui->lineEdit_Source->text());
        query.bindValue(":v3",ui->lineEdit_Destination->text());
        if(query.exec()){
            ui->listWidget_profils->addItem(ui->lineEdit_nomProfil->text());
            ui->lineEdit_nomProfil->clear();
            ui->listWidget_profils->setFixedHeight(ui->listWidget_profils->height()+20);
            ui->lineEdit_nomProfil->hide();
            ui->pushButton_annulerProfil->hide();
            ui->pushButton_nouveauProfil->setIcon(QIcon(":/Icon/Plus.png"));
        }
    }
}

void MainWindow::on_verifChamps()
{
    source = ui->lineEdit_Source->text();
    destination = ui->label_destination->text();
    ui->pushButton_Lancer->setEnabled(false);
    bool verif = (ui->lineEdit_Source->text()!="" && ui->lineEdit_Destination->text()!="");
    ui->pushButton_Contri->setEnabled(verif);
    ui->pushButton_Sync->setEnabled(verif);
    ui->pushButton_Echo->setEnabled(verif);
}

void MainWindow::on_pushButton_supprimerProfil_clicked()
{
    QMessageBox message;
    message.setWindowTitle("Suppression du profil");
    message.setText("Souhaitez-vous supprimer le profil : "+ui->listWidget_profils->currentItem()->text());
    message.addButton("Continuer",QMessageBox::AcceptRole);
    message.addButton("Annuler",QMessageBox::RejectRole);
    if(message.exec()==QMessageBox::AcceptRole){
        QSqlQuery query;
        query.prepare("DELETE FROM Profils where Nom=:nom;");
        query.bindValue(":nom",ui->listWidget_profils->currentItem()->text());
        if(query.exec())
            delete ui->listWidget_profils->currentItem();
    }
}

void MainWindow::on_listWidget_profils_currentTextChanged(const QString &currentText)
{
    QSqlQuery query;
    query.prepare("SELECT Source,Destination FROM Profils where Nom=:nom;");
    query.bindValue(":nom",currentText);
    if(query.exec())
    {
        while(query.next())
        {
            ui->lineEdit_Source->setText(query.value(0).toString());
            ui->lineEdit_Destination->setText(query.value(1).toString());
        }
    }
}

void MainWindow::on_pushButton_annulerProfil_clicked()
{
    ui->listWidget_profils->addItem(ui->lineEdit_nomProfil->text());
    ui->listWidget_profils->setFixedHeight(ui->listWidget_profils->height()+20);
    ui->lineEdit_nomProfil->hide();
    ui->pushButton_annulerProfil->hide();
    ui->pushButton_nouveauProfil->setEnabled(true);
    ui->pushButton_nouveauProfil->setIcon(QIcon(":/Icon/Plus.png"));
}

void MainWindow::on_lineEdit_nomProfil_textChanged(const QString &arg1)
{
    bool verif=true;
    for(int x=0;x<ui->listWidget_profils->count();x++){
        if(arg1==ui->listWidget_profils->item(x)->text()){
            verif=false;
        }
    }
    ui->pushButton_nouveauProfil->setEnabled(verif);
}
