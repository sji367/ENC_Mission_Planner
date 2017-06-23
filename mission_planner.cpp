#include "mission_planner.h"
#include "ui_mission_planner.h"

Mission_Planner::Mission_Planner(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Mission_Planner)
{
    GDALAllRegister();

    ui->setupUi(this);

    astar = A_Star(8); // Set the number of directions searched to 232
    geod = Geodesy();
    l84 = L84();

    feet2meters = 0.3048;
    grid_size = 5;
    MOOS_path = "~/moos-ivp/moos-ivp-reed";
    doneGridding = false;

    lat = 0;
    lon = 0;
    x_origin = 0;
    y_origin = 0;

    MLLW = 0;
    MHW = 0;

    ShipMeta = Vessel_Dimensions();
    desired_speed = 0;

    origin_set = false;
    WPT_set = false;
    ShipMeta_set = false;
    grid_built = false;
    ui->scrollArea_contents->findChild<QSlider*>("Mauniverablilty_slider")->setRange(0, 4); // Limit the range to 0-4

    gthread = new GriddingThread(this);
    qRegisterMetaType<vector<vector<int> > >("vector<vector<int> >");
    connect(gthread, SIGNAL(newGrid(vector<vector<int> >, double, double)), this, SLOT(onNewGrid(vector<vector<int> >, double, double)));
    connect(gthread, SIGNAL(StatusUpdate(QString, QString)), this, SLOT(onGridStatusUpdate(QString, QString)));

    origin_thread = new OriginThread(this);
    connect(origin_thread, SIGNAL(StatusUpdate(QString, QString)), this, SLOT(onOriginStatusUpdate(QString, QString)));
    connect(origin_thread, SIGNAL(newChart(QString, QString, double)), this, SLOT(onFoundENC(QString, QString, double)));
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
    origin_thread->setENCpath(MOOS_path+"/src/ENCs/");
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
            if (!valid_start)
            {
                cout << "Invalid Start Position." << endl;
                invalidWPT(WPT_list[i]);
                break;
            }
            if (!valid_finish)
            {
                cout << "Invalid Finish Position." << endl;
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
    cout << message.toStdString() << endl;
    ui->scrollArea_contents->findChild<QTextEdit*>("invalid_WPT_text")->setText(message);
}

void Mission_Planner::invalidWPT(const QString &WPT)
{
    QString message;
    message = "Invalid WPT: " + WPT;
    string a = message.toStdString();
    ui->scrollArea_contents->findChild<QTextEdit*>("invalid_WPT_text")->setText(QString::fromStdString(a));
}

// Grid the ENC at the desired size given by the
void Mission_Planner::on_setGridSizeButton_clicked()
{
    double MHW_offset;
    double buffer_dist;
    QStringList Chart;
    string chart_folder;

    // Parse the tide station text file for MLLW and MHW Datums
    getTideStationData();
    MHW_offset = MHW-MLLW;


    // Remove the .000 from the end of the file
    Chart = (QString::fromStdString(chart_name)).split(".");
    chart_folder = Chart[0].toStdString();
    string chart_path = MOOS_path+"/src/ENCs/"+chart_folder+"/"+chart_name;

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
    gthread->setMOOS_Path(MOOS_path+"/");
    gthread->setGeodesy(geod);
    gthread->setMHW_offset(MHW_offset);
}

void Mission_Planner::onGridStatusUpdate(QString status, QString color)
{
    ui->scrollArea_contents->findChild<QTextEdit*>("GriddingStatus")->setText(status);
    ui->scrollArea_contents->findChild<QTextEdit*>("GriddingStatus")->setStyleSheet(color);
}

void Mission_Planner::onOriginStatusUpdate(QString status, QString color)
{
    ui->scrollArea_contents->findChild<QTextEdit*>("OriginStatus")->setText(status);
    ui->scrollArea_contents->findChild<QTextEdit*>("OriginStatus")->setStyleSheet(color);
}

void Mission_Planner::onFoundENC(QString ENC_name, QString RNC_name, double scale)
{
    chart_name = ENC_name.toStdString();
    RNC_Name = RNC_name;
    setENC_Meta(ENC_name, QString::number(scale));
}

void Mission_Planner::on_Write_button_clicked()
{
    outfile_type = ui->scrollArea_contents->findChild<QComboBox*>("outFile_combobox")->currentText();
    QString filepath = ui->scrollArea_contents->findChild<QTextEdit*>("outfile_name")->toPlainText();

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
    newWindow.setStartPath("~/");
    newWindow.setModal(true);
    newWindow.UpdateLabel("Please click on the moos-ivp-reed directory.");
    if (newWindow.exec())
    {
        qsPath = newWindow.getPath();
    }
    ui->scrollArea_contents->findChild<QTextEdit*>("Path_text")->setText(qsPath);
    MOOS_path = qsPath.toStdString();
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
    newWindow.setStartPath(QString::fromStdString(MOOS_path));
    newWindow.setModal(true);
    newWindow.UpdateLabel("Please click on the directory in which\n you want the file to be stored.");
    if (newWindow.exec())
    {
        qsPath = newWindow.getPath();
    }
    ui->scrollArea_contents->findChild<QTextEdit*>("outfile_name")->setText(qsPath);
}

void Mission_Planner::getTideStationData()
{
    int cntr = 0;
    string tideStation_name = ui->scrollArea_contents->findChild<QComboBox*>("tideStation_combobox")->currentText().toStdString();
    QString tideStation_path = QString::fromStdString(MOOS_path+"/src/Tides/"+tideStation_name+"/"+tideStation_name+".txt");

    QStringList parsed;


    QFile file(tideStation_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd())
    {
        cntr++;
        if (cntr==5)
        {
            QString line = in.readLine();
            // Clear the old list
            parsed.clear();

            // Parse the file
            parsed = line.split("=");
            if(parsed.size() == 2)
            {
                MHW = parsed[1].toDouble();
                cout << "MHW = " << MHW << endl;
            }
        }
        else if (cntr==8)
        {
            QString line = in.readLine();
            // Clear the old list
            parsed.clear();

            // Parse the file
            parsed = line.split("=");
            if(parsed.size() == 2)
            {
                MLLW = parsed[1].toDouble();
                cout << "MLLW = " << MLLW << endl;
            }
        }
    }

}

void Mission_Planner::on_pushButton_clicked()
{
    RNC_Dialog RNC_planner;
    RNC_planner.setModal(true);
    RNC_planner.setMOOSPath(QString::fromStdString(MOOS_path));
    RNC_planner.setRNCName(RNC_Name);
    if (RNC_planner.exec())
        cout << "inside" << endl;

}
