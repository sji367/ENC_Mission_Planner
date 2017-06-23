#include "rnc_dialog.h"
#include "ui_rnc_dialog.h"

RNC_Dialog::RNC_Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RNC_Dialog)
{
    ui->setupUi(this);
    project = new AutonomousVehicleProject(this);

    ui->treeView->setModel(project->model());
    //ui->projectView->setStatusBar(statusBar());
    ui->projectView->setProject(project);

    ui->detailsView->setProject(project);
    connect(ui->treeView->selectionModel(),&QItemSelectionModel::currentChanged,ui->detailsView,&DetailsView::onCurrentItemChanged);

    connect(ui->projectView,&ProjectView::currentChanged,this,&RNC_Dialog::setCurrent);
}

RNC_Dialog::~RNC_Dialog()
{
    delete ui;
}

void RNC_Dialog::setCurrent(QModelIndex &index)
{
    ui->treeView->setCurrentIndex(index);
}

void RNC_Dialog::on_setRaster_PB_clicked()
{
    QString path = MOOSPath+"/src/RNCs/"+RNCName+"/"+RNCName;
    QString fname = QFileDialog::getOpenFileName(this,tr("Open"), path);
    if (!fname.isEmpty())
        project->openBackground(fname);
}

void RNC_Dialog::on_setTrackLine_PB_clicked()
{
    ui->projectView->setAddTracklineMode();
}

void RNC_Dialog::on_setWPT_PB_clicked()
{
    ui->projectView->setAddWaypointMode();
}
