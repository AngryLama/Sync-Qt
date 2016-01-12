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

void MainWindow::verifListes()
{
    if(fichiersVersDestination.length()==0 && repertoiresVersDestination.length()==0 && fichiersVersSource.length()==0 && repertoiresVersSource.length()==0 && fichiersSupprDestination.length()==0 && repertoiresSupprDestination.length()==0)
        ui->pushButton_Lancer->setEnabled(false);
    else ui->pushButton_Lancer->setEnabled(true);
}

void MainWindow::analyser()
{
    //Reset listes
    resetListes();
    fichiersSource.clear();
    fichiersDestination.clear();
    repertoiresSource.clear();
    repertoiresDestination.clear();

    changerMode(AUCUN);
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

void MainWindow::changerMode(Mode val)
{
    if(val!=AUCUN)
        mode=val;

    switch (val) {
    case ECHO:
        ui->pushButton_Contri->setEnabled(true);
        ui->pushButton_Sync->setEnabled(true);
        ui->pushButton_Echo->setEnabled(false);
        break;
    case CONTRIBUTION:
        ui->pushButton_Contri->setEnabled(false);
        ui->pushButton_Sync->setEnabled(true);
        ui->pushButton_Echo->setEnabled(true);
        break;
    case SYNCRONISATION:
        ui->pushButton_Contri->setEnabled(true);
        ui->pushButton_Sync->setEnabled(false);
        ui->pushButton_Echo->setEnabled(true);
        break;
    default:
        ui->pushButton_Contri->setEnabled(true);
        ui->pushButton_Sync->setEnabled(true);
        ui->pushButton_Echo->setEnabled(true);
        break;
    }
}

void MainWindow::analyseContribution()
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
    for(int x=0;x<repertoiresSource.length();x++){
        temp = destination+repertoiresSource[x];
        QDir dossier(temp);
        if(!dossier.isReadable()){
            tempI++;
            ui->tableWidget_Resume->item(0,1)->setText(QString::number(tempI));
            repertoiresVersDestination<<repertoiresSource[x];
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
    for(int x=0;x<repertoiresDestination.length();x++){
        temp = source+repertoiresDestination[x];
        QDir dossier(temp);
        if(!dossier.isReadable()){
            tempI++;
            ui->tableWidget_Resume->item(0,0)->setText(QString::number(tempI));
            repertoiresVersSource<<repertoiresDestination[x];
        }
    }
}

void MainWindow::analyseSyncronisation()
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
    for(int x=0;x<repertoiresSource.length();x++){
        temp = destination+repertoiresSource[x];
        QDir dossier(temp);
        if(!dossier.isReadable()){
            tempI++;
            ui->tableWidget_Resume->item(0,1)->setText(QString::number(tempI));
            repertoiresVersDestination<<repertoiresSource[x];
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
    for(int x=0;x<repertoiresDestination.length();x++){
        bool verif=false;
        for(int y=0;y<repertoiresSource.length();y++){
            if(repertoiresDestination[x]==repertoiresSource[y])
                verif=true;
        }
        if(!verif){
            tempI++;
            ui->tableWidget_Resume->item(0,2)->setText(QString::number(tempI));
            repertoiresSupprDestination<<repertoiresDestination[x];
        }
    }
}

void MainWindow::analyseEcho()
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
    for(int x=0;x<repertoiresDestination.length();x++){
        if(repertoiresSource.count(repertoiresDestination[x])==0){
            tempI++;
            ui->tableWidget_Resume->item(0,2)->setText(QString::number(tempI));
            repertoiresSupprDestination<<repertoiresDestination[x];
        }
    }

    //Analyse des répertoires à créer
    tempI=0;
    for(int x=0;x<repertoiresSource.length();x++){
        temp = destination+repertoiresSource[x];
        QDir dossier(temp);
        if(!dossier.isReadable()){
            tempI++;
            ui->tableWidget_Resume->item(0,1)->setText(QString::number(tempI));
            repertoiresVersDestination<<repertoiresSource[x];
        }
    }
}

void MainWindow::resetListes()
{
    //Reset listes
    fichiersVersDestination.clear();
    repertoiresVersDestination.clear();
    fichiersVersSource.clear();
    repertoiresVersSource.clear();
    fichiersSupprDestination.clear();
    repertoiresSupprDestination.clear();

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
    source = QFileDialog::getExistingDirectory(this, tr("Choisir le dossier source"),
                                                 "/home",
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
    if(source.length()>3)
        source+="/";
    ui->lineEdit_Source->setText(source);
}

void MainWindow::on_pushButton_Destination_clicked()
{
    destination = QFileDialog::getExistingDirectory(this, tr("Choisir le dossier source"),
                                                 "/home",
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
    if(destination.length()>3)
        destination+="/";
    ui->lineEdit_Destination->setText(destination);
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
    ui->label_fichierEnCours->clear();
    ui->label_fichierEnCours->show();
    ui->label_Wait->show();
    ui->pushButton_Lancer->setEnabled(false);
    ui->pushButton_Source->setEnabled(false);
    ui->pushButton_Destination->setEnabled(false);
    ui->pushButton_Contri->setEnabled(false);
    ui->pushButton_Sync->setEnabled(false);
    ui->pushButton_Echo->setEnabled(false);
    ui->pushButton_Quitter->setEnabled(false);
    ui->progressBar->setMaximum(fichiersSupprDestination.length()+repertoiresSupprDestination.length()+repertoiresVersSource.length()+repertoiresVersDestination.length()+fichiersVersSource.length()+fichiersVersDestination.length());
    QDir doss;

    //Suppression fichiers destination
    if(!fichiersSupprDestination.isEmpty()){
        for(int x=0;x<fichiersSupprDestination.length();x++){
            ui->progressBar->setValue(ui->progressBar->value()+1);
            doss.remove(destination+fichiersSupprDestination[x]);
        }
    }

    //Suppression dossiers destination
    if(!repertoiresSupprDestination.isEmpty()){
        for(int x=0;x<repertoiresSupprDestination.length();x++){
            ui->progressBar->setValue(ui->progressBar->value()+1);
            doss.rmdir(destination+repertoiresSupprDestination[x]);
        }
    }

    //Création dossiers vers source
    if(!repertoiresVersSource.isEmpty()){
        for(int x=0;x<repertoiresVersSource.length();x++){
            ui->progressBar->setValue(ui->progressBar->value()+1);
            doss.mkdir(source+repertoiresVersSource[x]);
        }
    }

    //Création dossiers vers destination
    if(!repertoiresVersDestination.isEmpty()){
        for(int x=0;x<repertoiresVersDestination.length();x++){
            ui->progressBar->setValue(ui->progressBar->value()+1);
            doss.mkdir(destination+repertoiresVersDestination[x]);
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
    repertoiresSource=liste;
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
    repertoiresDestination=liste;
    delete recherche[3];
    recherche[3]=NULL;
}

void MainWindow::on_finRecherche()
{
    if(recherche[0]==NULL && recherche[1]==NULL && recherche[2]==NULL && recherche[3]==NULL)
    {
        for(int x=0;x<fichiersSource.length();x++)
            fichiersSource[x].remove(source);
        for(int x=0;x<repertoiresSource.length();x++)
            repertoiresSource[x].remove(source);
        for(int x=0;x<fichiersDestination.length();x++)
            fichiersDestination[x].remove(destination);
        for(int x=0;x<repertoiresDestination.length();x++)
            repertoiresDestination[x].remove(destination);

        ui->progressBar->setMaximum(100);
        ui->progressBar->setValue(0);
        ui->label_Wait->hide();

        switch (mode) {
        case CONTRIBUTION:
            analyseContribution();
            break;
        case SYNCRONISATION:
            analyseSyncronisation();
            break;
        case ECHO:
            analyseEcho();
            break;
        default:
            break;
        }
        verifListes();
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
    ui->pushButton_Lancer->setEnabled(false);
    if(ui->lineEdit_Source->text()!="" && ui->lineEdit_Destination->text()!=""){
        ui->pushButton_Contri->setEnabled(true);
        ui->pushButton_Sync->setEnabled(true);
        ui->pushButton_Echo->setEnabled(true);
    }else{
        ui->pushButton_Contri->setEnabled(false);
        ui->pushButton_Sync->setEnabled(false);
        ui->pushButton_Echo->setEnabled(false);
    }
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
        query.prepare("DELETE FROM Profils where Nom=:nom");
        query.bindValue(":nom",ui->listWidget_profils->currentItem()->text());
        if(query.exec())
            delete ui->listWidget_profils->currentItem();
    }
}

void MainWindow::on_listWidget_profils_currentTextChanged(const QString &currentText)
{
    QSqlQuery query;
    query.prepare("SELECT Source,Destination FROM Profils where Nom=:nom");
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

void MainWindow::on_boutonsMode()
{
    QPushButton *temp=(QPushButton*)sender();
    if(temp==ui->pushButton_Contri)
        changerMode(CONTRIBUTION);
    if(temp==ui->pushButton_Sync)
        changerMode(SYNCRONISATION);
    if(temp==ui->pushButton_Echo)
        changerMode(ECHO);
    ui->label_Wait->hide();
    analyser();
}