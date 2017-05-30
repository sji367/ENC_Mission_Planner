#include "mission_planner.h"
#include "ui_mission_planner.h"

Mission_Planner::Mission_Planner(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Mission_Planner)
{
    ui->setupUi(this);

    astar = A_Star(1); // Set the number of directions searched to #
    geod = Geodesy();
    l84 = L84();

    feet2meters = 0.3048;
    grid_size = 5;
    sPath = "~/moos-ivp/moos-ivp-reed";
    doneGridding = false;

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
    ui->scrollArea_contents->findChild<QSlider*>("Mauniverablilty_slider")->setRange(0, 4); // Limit the range to 0-4

    gthread = new GriddingThread(this);
    qRegisterMetaType<QVector<int> >("QVector<int>");
    connect(gthread, SIGNAL(newGrid(QVector<int>)), this, SLOT(onNewGridRow(QVector<int>)));
    connect (gthread, SIGNAL(bounds(double, double)), this, SLOT(onGridBounds(double,double)));
    connect(gthread, SIGNAL(StatusUpdate(QString)), this, SLOT(onGridStatusUpdate(QString)));

    origin_thread = new OriginThread(this);
    connect(origin_thread, SIGNAL(StatusUpdate(QString)), this, SLOT(onOriginStatusUpdate(QString)));
    connect(origin_thread, SIGNAL(newChart(QString, double)), this, SLOT(onFoundENC(QString, double)));
}

Mission_Planner::~Mission_Planner()
{
    delete ui;
}

void Mission_Planner::getOrigin()
{
    lat = QString(ui->scrollArea_contents->findChild<QTextEdit*>("lat_origin_in")->toPlainText()).toDouble();
    lon = QString(ui->scrollArea_contents->findChild<QTextEdit*>("lon_origin_in")->toPlainText()).toDouble();
    geod.Initialise(lat,lon);
}

void Mission_Planner::getShipMeta()
{
    double length, width, draft, turn_radius;
    QString length_unit, speed_unit;
    length = QString(ui->scrollArea_contents->findChild<QTextEdit*>("ship_length_in")->toPlainText()).toDouble();
    width = QString(ui->scrollArea_contents->findChild<QTextEdit*>("ship_width_in")->toPlainText()).toDouble();
    draft = QString(ui->scrollArea_contents->findChild<QTextEdit*>("ship_draft_in")->toPlainText()).toDouble();
    desired_speed = QString(ui->scrollArea_contents->findChild<QTextEdit*>("desired_speed_in")->toPlainText()).toDouble();
    length_unit = ui->scrollArea_contents->findChild<QComboBox*>("unit_combobox")->currentText();
    speed_unit = ui->scrollArea_contents->findChild<QComboBox*>("speedUnit_combobox")->currentText();

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
    turn_radius = (5-ui->scrollArea_contents->findChild<QSlider*>("Mauniverablilty_slider")->value())*length;

    // Set the ship's meta data
    astar.setDesiredSpeed(desired_speed);
    ShipMeta.set_ship_meta(length, width, draft, turn_radius);
    astar.setShipMeta(ShipMeta);

}

void Mission_Planner::setENC_Meta(const QString &ENC, const QString &Scale)
{
    ui->scrollArea_contents->findChild<QTextEdit*>("ENC_text")->setText(ENC);
    ui->scrollArea_contents->findChild<QTextEdit*>("scale_text")->setText(Scale);
}

void Mission_Planner::on_setOriginButton_clicked()
{
    getOrigin();

    // Find the ENC with the smallest scale that includes the origin
    origin_thread->start();
    origin_thread->setLatLong(lat,lon);
    origin_thread->setENCpath(sPath+"/src/ENCs/");


}

void Mission_Planner::on_setShipMeta_clicked()
{
    getShipMeta();
}

void Mission_Planner::on_AStar_Button_clicked()
{
    QTime t;
    int num_WPTs =0;
    QString WPT, new_WPTs;
    QStringList WPT_list, start_xy, finish_xy;
    bool valid_start, valid_finish;

    t.start();

    // Parse WPTs
    WPT= ui->scrollArea_contents->findChild<QTextEdit*>("waypoints_in")->toPlainText();
    WPT_list = WPT.split(":");
    num_WPTs = WPT_list.size();

    // build grid
    astar.setConversionMeta(grid_size, MinX, MinY);
    astar.setMap(Map);
    //astar.build_map(sPath+"/src/ENCs/Shape/grid/grid.csv");

    for (int i=0; i<num_WPTs-1; i++)
    {
        // Get start/finsih WPTs
        start_xy = WPT_list[i].split(",");
        finish_xy = WPT_list[i+1].split(",");
        astar.setStartFinish_Grid(start_xy[i].toDouble(), start_xy[i+1].toDouble(), finish_xy[i].toDouble(),finish_xy[i+1].toDouble());

        astar.getNM();
        printf("Start: %i, %i\t Finish: %i, %i\n", astar.getStartX(), astar.getStartY(), astar.getFinishX(), astar.getFinishY());

        // Check waypoints
        astar.checkStartFinish();
        valid_start = astar.getStartValid();
        valid_finish = astar.getFinishValid();

        // If the waypoints are valid, run A*
        if ((valid_start)&&(valid_finish))
        {
            cout << "WPTs are valid\n";
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
            cout << "Invalid" << endl;
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
        cout << t.elapsed() << " ms" << endl;
    }
    ui->scrollArea_contents->findChild<QTextEdit*>("invalid_WPT_text")->setText(allWPTs);
}
void Mission_Planner::NoPathFound(const QString &startWPT, const QString &endWPT)
{
    QString message;
    message = "No path found between " + startWPT + " and " + endWPT;
    ui->scrollArea_contents->findChild<QTextEdit*>("invalid_WPT_text")->setText(message);
}

void Mission_Planner::invalidWPT(const QString &WPT)
{
    QString message;
    message = "Invalid WPT: " + WPT;
    ui->scrollArea_contents->findChild<QTextEdit*>("invalid_WPT_text")->setText(message);
}

// Grid the ENC at the desired size given by the
void Mission_Planner::on_setGridSizeButton_clicked()
{
    double buffer_dist;
    QStringList Chart;
    string chart_folder;

    // Remove the .000 from the end of the file
    Chart = (QString::fromStdString(chart_name)).split(".");
    chart_folder = Chart[0].toStdString();
    string chart_path = sPath+"/src/ENCs/"+chart_folder+"/"+chart_name;

    // Assume that the ASV is a rectange and the buffer distance is the length of the diagonal
    buffer_dist = sqrt(pow(ShipMeta.getLength(), 2) + pow(ShipMeta.getWidth(), 2));

    // Take the input from the text edit
    grid_size=QString(ui->scrollArea_contents->findChild<QTextEdit*>("grid_size_in")->toPlainText()).toDouble();

    // If the grid size is bigger than the maximum size of the ASV, set the buffer distance
    //  for all polygons in the grid to the grid size.
    if (buffer_dist < grid_size)
        buffer_dist = grid_size;

    // Start a thread to do all of the gridding
    gthread->start();
    gthread->setGridSize(grid_size);
    gthread->setChartPath(chart_path);
    gthread->setBufferSize(buffer_dist);
    gthread->setMOOS_Path(sPath+"/");
    gthread->setGeodesy(geod);
}

void Mission_Planner::onNewGridRow(QVector<int> MAP)
{
    Map.push_back(MAP.toStdVector());
}

void Mission_Planner::onGridStatusUpdate(QString status)
{
    ui->scrollArea_contents->findChild<QTextEdit*>("GriddingStatus")->setText(status);
}

void Mission_Planner::onOriginStatusUpdate(QString status)
{
    ui->scrollArea_contents->findChild<QTextEdit*>("OriginStatus")->setText(status);
}

void Mission_Planner::onFoundENC(QString name, double scale)
{
    chart_name = name.toStdString();
    setENC_Meta(name, QString::number(scale));
}

void Mission_Planner::on_Write_button_clicked()
{
    outfile_type = ui->scrollArea_contents->findChild<QComboBox*>("outFile_combobox")->currentText();
    QString filepath = ui->scrollArea_contents->findChild<QTextEdit*>("outfile_name")->toPlainText();
    cout << filepath.toStdString() << ", " << allWPTs.toStdString() << endl;

    if (outfile_type=="MOOS")
    {
        cout << "MOOS" << endl;
        astar.buildMOOSFile(filepath.toStdString(), allWPTs.toStdString());
    }
    else if (outfile_type == "L84")
    {
        l84 = L84(filepath.toStdString(), allWPTs.toStdString(), lat, lon);
    }
    cout << "Built File" << endl;
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
    ui->scrollArea_contents->findChild<QTextEdit*>("Path_text")->setText(qsPath);
    sPath = qsPath.toStdString();
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
    ui->scrollArea_contents->findChild<QTextEdit*>("outfile_name")->setText(qsPath);
}
