#ifndef MISSION_PLANNER_H
#define MISSION_PLANNER_H

#include <QMainWindow>
#include <QTime>
#include <string>
#include "ogrsf_frmts.h" // for gdal/ogr
#include "ogr_spatialref.h"
#include "astar.h"
#include "L84.h"
#include "geodesy.h"
#include <vector>
#include <stdlib.h>
#include <boost/filesystem.hpp>
#include "ENC_picker.h"
#include "filedialog.h"
#include "gridinterp.h"
#include <cmath>
#include "griddingthread.h"
#include "originthread.h"
#include <QVector>

namespace fs = ::boost::filesystem;

namespace Ui {
class Mission_Planner;
}

class Mission_Planner : public QMainWindow
{
    Q_OBJECT

public:
    explicit Mission_Planner(QWidget *parent = 0);
    ~Mission_Planner();

    void getOrigin();
    void getShipMeta();
    void setENC_Meta(const QString &ENC, const QString &Scale);
    void NoPathFound(const QString &startWPT, const QString &endWPT);
    void invalidWPT(const QString &WPT);
    double dist(int x1, int y1, int x2, int y2);

public slots:
    // Gridding thread
    void onNewGridRow(QVector<int> MAP);
    void onGridBounds(double MINX, double MINY) {MinX = MINX, MinY = MINY; }
    void onGridStatusUpdate(QString status);

    // Origin Thread
    void onFoundENC(QString name, double scale);
    void onOriginStatusUpdate(QString status);

private slots:
    void on_setOriginButton_clicked();
    void on_setShipMeta_clicked();
    void on_AStar_Button_clicked();
    void on_setGridSizeButton_clicked();
    void on_Write_button_clicked();
    void on_Path_pushButton_clicked();
    void on_OutfilePath_pushButton_clicked();

private:
    Ui::Mission_Planner *ui;
    A_Star astar;
    L84 l84;
    Geodesy geod;
    Vessel_Dimensions ShipMeta;
    //Grid_Interp grid;
    vector<vector<int> > Map;
    double feet2meters;
    double lat, lon, x_origin, y_origin;
    double MinX, MinY;
    double grid_size;
    double desired_speed;
    bool origin_set, ShipMeta_set, WPT_set, grid_built;
    QString outfile_type, output_path, moos_path, allWPTs;
    string sPath, chart_name;
    bool doneGridding;
    GriddingThread *gthread;
    OriginThread *origin_thread;
};

#endif // MISSION_PLANNER_H
