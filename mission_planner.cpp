#include "mission_planner.h"
#include "ui_mission_planner.h"

Mission_Planner::Mission_Planner(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Mission_Planner)
{
    ui->setupUi(this);

    astar = A_Star(8); // Set the number of directions searched to #
    geod = Geodesy();
    l84 = L84();
    //grid  = Grid_ENC();

    feet2meters = 0.3048;
    grid_size = 5;
    sPath = "~/moos-ivp/moos-ivp-reed";

    lat = 0;
    lon = 0;
    x_origin = 0;
    y_origin = 0;

    ShipMeta = Vessel_Dimensions();
    desired_speed = 0;

    origin_set = false;
    WPT_set = false;
    ShipMeta_set = false;
    grid_built = false;
    ui->Mauniverablilty_slider->setRange(0, 4); // Limit the range to 0-4
}

Mission_Planner::~Mission_Planner()
{
    delete ui;
}

void Mission_Planner::getOrigin()
{
    lat = QString(ui->lat_origin_in->toPlainText()).toDouble();
    lon = QString(ui->lon_origin_in->toPlainText()).toDouble();
    geod.Initialise(lat,lon);
}

void Mission_Planner::getShipMeta()
{
    double length, width, draft, turn_radius;
    QString length_unit, speed_unit;
    length = QString(ui->ship_length_in->toPlainText()).toDouble();
    width = QString(ui->ship_width_in->toPlainText()).toDouble();
    draft = QString(ui->ship_draft_in->toPlainText()).toDouble();
    desired_speed = QString(ui->desired_speed_in->toPlainText()).toDouble();
    length_unit = ui->unit_combobox->currentText();
    speed_unit = ui->speedUnit_combobox->currentText();

    // Convert length measurements to meters
    if (length_unit == "Feet")
    {
        length *= feet2meters;
        width  *= feet2meters;
        draft  *= feet2meters;
    }

    // convert speed to m/s
    if (speed_unit== "ft/s")
        desired_speed *= feet2meters;
    if  (speed_unit =="knots")
        desired_speed *= 463.0/900.0;

    // Calculate turn radius
    turn_radius = (5-ui->Mauniverablilty_slider->value())*length;

    // Set the ship's meta data
    astar.setDesiredSpeed(desired_speed);
    ShipMeta.set_ship_meta(length, width, draft, turn_radius);
    astar.setShipMeta(ShipMeta);

}

void Mission_Planner::setENC_Meta(const QString &ENC, const QString &Scale)
{
    ui->ENC_text->setText(ENC);
    ui->scale_text->setText(Scale);
}

void Mission_Planner::on_setOriginButton_clicked()
{
    int chart_scale = -2;
    getOrigin();
    ENC_Picker picker = ENC_Picker(lat,lon, sPath+"/src/ENCs/");
    picker.pick_ENC(chart_name, chart_scale);
    setENC_Meta(QString::fromStdString(chart_name), QString::number(chart_scale));
}

void Mission_Planner::on_setShipMeta_clicked()
{
    getShipMeta();
}

void Mission_Planner::on_AStar_Button_clicked()
{
    int num_WPTs =0;
    QString WPT, new_WPTs;
    QStringList WPT_list, start_xy, finish_xy;
    bool valid_start, valid_finish;

    // Parse WPTs
    WPT= ui->waypoints_in->toPlainText();
    WPT_list = WPT.split(":");
    num_WPTs = WPT_list.size();

    // build grid
    //vector<vector<int> > ENCmap = grid.getGriddedMap();
    //astar.build_map(ENCmap);
    // NEED TO ADD SOMETHING THAT ACTUALLY DOES THIS
    astar.build_map(sPath+"/src/ENCs/Shape/grid/grid.csv");

    for (int i=0; i<num_WPTs-1; i++)
    {
        // Get start/finsih WPTs
        start_xy = WPT_list[i].split(",");
        finish_xy = WPT_list[i+1].split(",");
        astar.setStartFinish(start_xy[i].toDouble(), start_xy[i+1].toDouble(), finish_xy[i].toDouble(),finish_xy[i+1].toDouble());
        
        // Check waypoints
        astar.checkStartFinish();
        valid_start = astar.getStartValid();
        valid_finish = astar.getFinishValid();

        // If the waypoints are valid, run A*
        if ((valid_start)&&(valid_finish))
        {
            new_WPTs = QString::fromStdString(astar.AStar_Search());
            if (new_WPTs.isEmpty())
            {
                NoPathFound(WPT_list[i], WPT_list[i+1]);
            }
            else
                allWPTs += new_WPTs;
        }
        // Otherwise print an error message and quit out of the loop
        else
        {
            if (!valid_start)
            {
                invalidWPT(WPT_list[i]);
                break;
            }
            if (!valid_finish)
            {
                invalidWPT(WPT_list[i+1]);
                break;
            }
        }

        // else print invalid wpts on screen
    }
    ui->invalid_WPT_text->setText(allWPTs);
}
void Mission_Planner::NoPathFound(const QString &startWPT, const QString &endWPT)
{
    QString message;
    message = "No path found between " + startWPT + " and " + endWPT;
    ui->invalid_WPT_text->setText(message);
}

void Mission_Planner::invalidWPT(const QString &WPT)
{
    QString message;
    message = "Invalid WPT: " + WPT;
    ui->invalid_WPT_text->setText(message);
}

// Grid the ENC at the desired size given by the
void Mission_Planner::on_setGridSizeButton_clicked()
{
    double buffer_dist;
    // Assume that the ASV is a rectange and the buffer distance is the length of the diagonal
    buffer_dist = sqrt(pow(ShipMeta.getLength(), 2) + pow(ShipMeta.getWidth(), 2));

    grid_size=QString(ui->ship_length_in->toPlainText()).toDouble();
    //grid  = Grid_ENC(chart_name, grid_size, buffer_dist, geod);

    // Make the grid for the ASV
    //grid.MakeBaseMap(grid_size/2);
}


void Mission_Planner::on_Write_button_clicked()
{
    outfile_type = ui->unit_combobox->currentText();
    QString filepath = ui->outfile_name->toPlainText();
    if (outfile_type=="MOOS")
        astar.buildMOOSFile(sPath+"/missions/", allWPTs.toStdString());
    else if (outfile_type == "L84")
    {
        l84 = L84(sPath+"/missions/", allWPTs.toStdString(), lat, lon);
    }

}

// Get the user to input the location of Extended MOOS-IvP (moos-ivp-reed)
//  When the button is pressed, a new window will open and the user will navigate
//  through the file system until they find the moos-ivp-reed window.
void Mission_Planner::on_Path_pushButton_clicked()
{
    QString qsPath;
    fileDialog newWindow;
    newWindow.setModal(true);
    newWindow.UpdateLabel("Please click on the moos-ivp-reed directory.");
    if (newWindow.exec())
    {
        qsPath = newWindow.getPath();
    }
    ui->Path_text->setText(qsPath);
    sPath = qsPath.toStdString();
}

void Mission_Planner::on_pushButton_clicked()
{
    vector<vector<int> > Map(10, vector<int>(10, -15));
    OGRPolygon *poly;
    OGRLinearRing *ring;
    OGRPoint *point;
    vector<int> x,y,z;
    vector<double> d;
    int Z;
    std::ifstream in("/home/sji367/moos-ivp/moos-ivp-reed/src/lib_ENC_util/terrain.cin");
    std::istream_iterator<Point> begin(in);
    std::istream_iterator<Point> end;
    Delaunay dt(begin, end);

    int minX, maxX, maxY, minY;


    for( Delaunay::Finite_faces_iterator fi = dt.finite_faces_begin(); fi != dt.finite_faces_end(); fi++)
      {
        x.clear(); y.clear(); z.clear();

        for(int i=0; i<3; i++)
        {
            x.push_back(fi->vertex(i)->point().hx());
            y.push_back(fi->vertex(i)->point().hy());
            z.push_back(fi->vertex(i)->point().hz());

            Map[y[i]][x[i]] = z[i]*10;
        }
        // Build the polygon of the Delaunay triangle
        poly = (OGRPolygon*) OGRGeometryFactory::createGeometry(wkbPolygon);
        ring = (OGRLinearRing *) OGRGeometryFactory::createGeometry(wkbLinearRing);
        ring->addPoint(x[0],y[0]);
        ring->addPoint(x[1],y[1]);
        ring->addPoint(x[2],y[2]);
        ring->addPoint(x[0],y[0]);

        ring->closeRings();
        poly->addRing(ring);
        poly->closeRings();

        minX = *min_element(x.begin(), x.end());
        maxX = *max_element(x.begin(), x.end());

        minY = *min_element(y.begin(), y.end());
        maxY = *max_element(y.begin(), y.end());

        // Now cycle though the points to get
        for(int gridX=minX; gridX<= maxX; gridX++)
        {
            for (int gridY=minY; gridY<= maxY; gridY++)
            {
                point = (OGRPoint*) OGRGeometryFactory::createGeometry(wkbPoint);
                point->setX(gridX);
                point->setY(gridY);

                if (point->Intersects(poly))//||point->Touches(poly))
                {
                    d.clear();
                    // Calculate the distance to each vertex
                    d.push_back(dist(x[0],y[0], gridX,gridY));
                    d.push_back(dist(x[1],y[1], gridX,gridY));
                    d.push_back(dist(x[2],y[2], gridX,gridY));
                    if((d[0]!=0) && (d[1]!=0) && (d[2]!=0))
                    {
                        Z = int(round((1.0*z[0]/d[0]+ 1.0*z[1]/d[1] +1.0*z[2]/d[2])/(1.0/d[0]+1.0/d[1]+1.0/d[2])*10));
                        Map[gridY][gridX]= Z;
                    }
                }
          }
        }

      }
    for (unsigned i = 0; i<Map.size(); i++){
      for (unsigned j=0; j<Map[0].size(); j++)
        {
      cout << Map[i][j] << ",\t";
        }
      cout << endl;
    }
}

double Mission_Planner::dist(int x1, int y1, int x2, int y2)
{
    return (pow((x1-x2),2)+pow((y1-y2),2));
}

void Mission_Planner::on_OutfilePath_pushButton_clicked()
{
    // Get the user to input the location of Extended MOOS-IvP (moos-ivp-reed)
    //  When the button is pressed, a new window will open and the user will navigate
    //  through the file system until they find the moos-ivp-reed window.
    QString qsPath;
    fileDialog newWindow;
    newWindow.setModal(true);
    newWindow.UpdateLabel("Please click on the directory in which\n you want the file to be stored.");
    if (newWindow.exec())
    {
        qsPath = newWindow.getPath();
    }
    ui->outfile_name->setText(qsPath);
}
