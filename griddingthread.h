#ifndef GRIDDINGTHREAD_H
#define GRIDDINGTHREAD_H

#include <QThread>
#include <QtCore>
#include <QVector>
#include "astar.h"
#include "gridinterp.h"
#include <vector>

class GriddingThread : public QThread
{
    Q_OBJECT
public:
    explicit GriddingThread(QObject *parent = 0);
    void run();
    bool stop;

    void setGridSize(double gridSize) {grid_size = gridSize; }
    void setChartPath(string chartPath) {chart_path = chartPath; }
    void setBufferSize(double buffer) {buffer_dist = buffer; }
    void setMOOS_Path(string MOOS) { MOOS_Path = MOOS; }
    void setGeodesy(Geodesy Geod) { geod = Geodesy(Geod.getLatOrigin(), Geod.getLonOrigin()); }


signals:
    void newGrid(QVector<int>);
    void bounds(double, double);
    void StatusUpdate(QString status);

private:
    double grid_size, buffer_dist;
    string chart_path, MOOS_Path;
    Geodesy geod;

};

#endif // GRIDDINGTHREAD_H
