#include "griddingthread.h"

GriddingThread::GriddingThread(QObject *parent) : QThread(parent)
{
    grid_size = 5;
    buffer_dist = 0;
    geod = Geodesy();
    MHW_Offset = -1;

}

void GriddingThread::run()
{
    emit StatusUpdate("Starting Gridding...", "QTextEdit{color: red;}");

    QMutex mutex;
    mutex.lock();

    Grid_Interp grid = Grid_Interp(MOOS_Path, chart_path, grid_size, buffer_dist, MHW_Offset, geod);
    grid.Run(false);

    mutex.unlock();

    emit newGrid(grid.transposeMap(), grid.getMinX(), grid.getMinY());
    emit StatusUpdate("Finished Gridding!", "QTextEdit{color: green;}");
}





