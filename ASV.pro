#-------------------------------------------------
#
# Project created by QtCreator 2017-04-08T16:47:11
#
#-------------------------------------------------

QT += core gui positioning

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ASV
TEMPLATE = app

SOURCES += main.cpp\
        mission_planner.cpp \
    filedialog.cpp \
    griddingthread.cpp \
    originthread.cpp \
    rnc_dialog.cpp \
    autonomousvehicleproject.cpp \
    backgroundraster.cpp \
    detailsview.cpp \
    georeferenced.cpp \
    waypoint.cpp \
    projectview.cpp \
    trackline.cpp \
    geographicsitem.cpp \
    surveypattern.cpp \
    waypointdetails.cpp \
    tracklinedetails.cpp \
    surveypatterndetails.cpp

HEADERS  += mission_planner.h \
    filedialog.h \
    griddingthread.h \
    originthread.h \
    rnc_dialog.h \
    autonomousvehicleproject.h \
    backgroundraster.h \
    detailsview.h \
    georeferenced.h \
    waypoint.h \
    projectview.h \
    trackline.h \
    geographicsitem.h \
    surveypattern.h \
    waypointdetails.h \
    tracklinedetails.h \
    surveypatterndetails.h

FORMS    += mission_planner.ui \
    filedialog.ui \
    rnc_dialog.ui \
    waypointdetails.ui \
    tracklinedetails.ui \
    surveypatterndetails.ui

# Include GDAL
INCLUDEPATH += "/usr/include/"
LIBS += "-lgdal"

#Include MOOS stuff
#INCLUDEPATH += "~/moos-ivp/lib"
#LIBS += -L/home/sji367/moos-ivp/lib
#LIBS += -lmoos

INCLUDEPATH += "/home/sji367/moos-ivp/moos-ivp-reed/src/lib_ENC_util/"
LIBS += -L/home/sji367/moos-ivp/moos-ivp-reed/lib/
LIBS += -lENC_util
LIBS += "-lENCGrid"

# Include boost
LIBS += -lboost_filesystem -lboost_system

QMAKE_CXXFLAGS += -std=c++11
