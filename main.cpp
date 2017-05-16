#include "mission_planner.h"
#include <QApplication>
#include "astar.h"
#include "geodesy.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Mission_Planner w;
    w.show();

    return a.exec();
}
