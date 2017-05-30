#ifndef ORIGINTHREAD_H
#define ORIGINTHREAD_H

#include <QObject>
#include <string>
#include <QThread>
#include <QtCore>
#include "ENC_picker.h"

using namespace std;

class OriginThread : public QThread
{
    Q_OBJECT
public:
    OriginThread(QObject *parent = 0);
    void run();

    void setLatLong(double Lat, double Lon) {lat = Lat; lon = Lon; }
    void setENCpath(string ENC) { ENC_Path = ENC; }

signals:
    void newChart(QString, double);
    void StatusUpdate(QString status);

private:
    double lat, lon;
    string ENC_Path;
};

#endif // ORIGINTHREAD_H
