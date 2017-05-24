#ifndef MISSION_PLANNER_H
#define MISSION_PLANNER_H

#include <QMainWindow>
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
#include "gridENC.h"
#include "tin_enc.h"
#include <cmath>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Projection_traits_xy_3.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <fstream>
#include <vector>
#include "gdal_frmts.h" // for GDAL/OGR
typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Projection_traits_xy_3<K>  Gt;
typedef CGAL::Delaunay_triangulation_2<Gt> Delaunay;
typedef K::Point_3   Point;


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

private slots:
    void on_setOriginButton_clicked();
    void on_setShipMeta_clicked();
    void on_AStar_Button_clicked();
    void on_setGridSizeButton_clicked();
    void on_Write_button_clicked();
    void on_Path_pushButton_clicked();

    void on_pushButton_clicked();

    void on_OutfilePath_pushButton_clicked();

private:
    Ui::Mission_Planner *ui;
    A_Star astar;
    L84 l84;
    Geodesy geod;
    TIN_ENC grid;
    Vessel_Dimensions ShipMeta;

    double feet2meters;
    double lat, lon, x_origin, y_origin;
    qint8 grid_size;
    double desired_speed;
    bool origin_set, ShipMeta_set, WPT_set, grid_built;
    QString outfile_type, output_path, moos_path, allWPTs;
    std::string sPath, chart_name;
};

#endif // MISSION_PLANNER_H
