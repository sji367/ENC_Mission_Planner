#include "filedialog.h"
#include "ui_filedialog.h"
#include <iostream>

fileDialog::fileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::fileDialog)
{
    ui->setupUi(this);

    sPath = "~/";
    dirmodel = new QFileSystemModel(this);
    dirmodel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);
    dirmodel->setRootPath(sPath);
    ui->path_treeView->setModel(dirmodel);
}

fileDialog::~fileDialog()
{
    delete ui;
}

void fileDialog::on_path_treeView_clicked(const QModelIndex &index)
{
    sPath = dirmodel->fileInfo(index).absoluteFilePath();
}

void fileDialog::UpdateLabel(const QString text)
{
    ui->label->setText(text);
}
