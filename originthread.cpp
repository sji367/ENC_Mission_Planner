#include "originthread.h"

OriginThread::OriginThread(QObject *parent) : QThread(parent)
{
    lat= 43.071959;
    lon = -70.711611;
}


void OriginThread::run()
{
    emit StatusUpdate("Picking the ENC...");

    int chart_scale = -2;
    string chart_name;

    ENC_Picker picker = ENC_Picker(lat,lon, ENC_Path);
    picker.pick_ENC(chart_name, chart_scale);

    emit newChart(QString::fromStdString(chart_name), chart_scale);
    emit StatusUpdate("Found the ENC!");

}
