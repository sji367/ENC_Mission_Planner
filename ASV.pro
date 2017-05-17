#-------------------------------------------------
#
# Project created by QtCreator 2017-04-08T16:47:11
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ASV
TEMPLATE = app


SOURCES += main.cpp\
        mission_planner.cpp \
    filedialog.cpp

HEADERS  += mission_planner.h \
    filedialog.h

FORMS    += mission_planner.ui \
    filedialog.ui

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

# Include boost
LIBS += -lboost_filesystem -lboost_system

# CGAL
INCLUDEPATH +=   /usr/include/
LIBS        += -L/usr/include/
LIBS        += -lCGAL
LIBS        += -lgmp
LIBS        += -lmpfr
QMAKE_CXXFLAGS += -frounding-math -O3


QMAKE_CXXFLAGS += -std=c++11
