#ifndef RNC_DIALOG_H
#define RNC_DIALOG_H

#include <QDialog>
#include <QModelIndex>

#include <QFileDialog>
#include <QStatusBar>
#include <QStandardItemModel>
#include <gdal_priv.h>
#include <cstdint>

#include "autonomousvehicleproject.h"
#include "waypoint.h"

namespace Ui {
class RNC_Dialog;
}

class RNC_Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit RNC_Dialog(QWidget *parent = 0);
    void setMOOSPath(QString sPath) { MOOSPath = sPath; }
    void setRNCName(QString name) { RNCName = name; }
    ~RNC_Dialog();

public slots:
    void setCurrent(QModelIndex &index);

private slots:
    //void on_setRaster_PB_clicked();
    void on_setWPT_PB_clicked();
    void on_setTrackLine_PB_clicked();

    void on_setRaster_PB_clicked();

private:
    Ui::RNC_Dialog *ui;
    QString MOOSPath, RNCName;

    AutonomousVehicleProject *project;
};

#endif // RNC_DIALOG_H
