/* FTSPlot - fast time series dataset plotter
   Copyright (C) 2013  Michael Riss <Michael.Riss@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA */


#ifndef __SIMPLEVIEWWIDGET_H__
#define __SIMPLEVIEWWIDGET_H__

namespace FTSPlot
{

class SimpleViewWidget;

}

#include <QSet>
#include <QOpenGLWidget>
#if defined(Q_WS_MAC)
# include <OpenGL/glu.h>
#else
# ifndef QT_LINUXBASE
#  include <GL/glu.h>
# endif
#endif
#include <QThread>
#include <QFileDialog>
#include <QMutex>
#include <QPoint>
#include <QWaitCondition>
#include <QMouseEvent>
#include <QColor>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "GL_Layer.h"
#include "SimpleViewModulesModel.h"

namespace FTSPlot
{

// add some missing functions (if they are missing)
#ifndef round
inline double round ( double x ) {
    return floor ( x + 0.5 );
}
#endif

#ifndef log2
inline double log2 ( double n ) {
    return log ( n ) / log ( ( double ) 2.0 );
}
#endif

class vizModule
{
public:
    GL_Layer* module;
    QString name;
    QColor color;
    bool visible;
    bool editable;
};

class SimpleViewWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    SimpleViewWidget();
    SimpleViewWidget ( QWidget* parent );
    ~SimpleViewWidget();
    long double Xscreen2graph ( int x );
    double Yscreen2graph ( int y );
    int getWindowHeight();
    SimpleViewModulesModel* dataModel();
    GL_Layer* addTimeSeries ( QString cfgFileName );
    QString cachedTimeSeriesDirectory;
    GL_Layer* addEventEditorModule ( QString TreeDirName );
    GL_Layer* addIntervalEditorModule ( QString TreeDirName );
    QList<vizModule> listModules();
    bool deleteModule ( GL_Layer* mod );
    bool showModule ( GL_Layer* mod );
    bool hideModule ( GL_Layer* mod );
    bool isModuleVisible ( GL_Layer* mod );
    bool showModuleGUI ( GL_Layer* mod );
    bool hideModuleGUI( GL_Layer* mod );
    bool isModuleGUIVisible( GL_Layer* mod );
    QColor getModuleColor ( GL_Layer* mod );
    bool setModuleColor ( GL_Layer* mod, QColor col );
    GL_Layer* getEditModule();
    bool setEditModule ( GL_Layer* mod );
    QString getModuleName( GL_Layer* mod );
    bool setModuleName( GL_Layer* mod, QString name );
    void setXRange ( long double Xbegin, long double Xend );
    QPair<long double, long double> getXRange();
    void setYRange ( double Ymin, double Ymax );
    QPair<double, double> getYRange();
    bool reorderModules ( QList<vizModule> orderedMods );

protected:
    void initializeGL();
    void resizeGL ( int w, int h );
    void paintGL();
    void mousePressEvent ( QMouseEvent* );
    void mouseMoveEvent ( QMouseEvent* );
    void wheelEvent ( QWheelEvent *event );
    void keyPressEvent ( QKeyEvent * event );
    void keyReleaseEvent ( QKeyEvent * event );
    void enterEvent ( QEvent * event );
    void leaveEvent ( QEvent * event );

private:
    friend class SimpleViewModulesModel;
    GL_Layer* activeInputModule;
    void setupViewportProjection();
    void requestNewLists();
    void requestNewList ( GL_Layer* mod );
    void updateVizList();
    int recordFile;
    int recordFile64;
    void* recordMem;
    void* recordMem64;
    size_t recordLength;
    size_t recordLength64;
    double recordMax;
    double recordMin;
    int MouseX;
    int MouseY;
    qint64 reqXdataBegin;
    qint64 reqXdataEnd;
    qint64 XdataBegin;
    qint64 XdataEnd;
    unsigned int reqPower;
    unsigned int power;
    unsigned int reqDispPower;
    unsigned int dispPower;
    long double Xcursor;
    double Xscale;
    double Ymin;
    double Ymax;
    double YFrustMax;
    double YFrustMin;
    double reqYFrustMax;
    double reqYFrustMin;
    GLint windowWidth;
    GLint windowHeight;
    GLboolean doRender;
    bool filesready;
    GLuint displayLists[2];
    QList<GL_Layer*> useList;
    QSet<GL_Layer*> reqSet;
    QList<GL_Layer*> reqList;
    QList<GL_Layer*> vizList;
    QList<GL_Layer*> updateList;
    QList<vizModule> modules;
    bool changeDispListSet;
    bool requested;
    bool updateNeeded;
    double Xgraph2GL ( long double );
    static Qt::GlobalColor stdColors[];
    int colorIdx;
    int NoStdColors;
    GL_Layer* genEventEditor();
    GL_Layer* genIntervalEditor();

    long double crossHairX;
    double crossHairY;
    long double Xmin;
    long double Xmax;

    SimpleViewModulesModel svm;

    int EventListCounter;
    int IntervalListCounter;
    
#ifdef BENCHMARK
    struct timespec displayList_startTime;
#endif // BENCHMARK

signals:
    void writeCrossCoords ( const QString &message, int timeout );
    void clearCrossCoords();
    void newCoords ( long double Xcursor, double Xscale, double Ymin, double Ymax );

#ifdef BENCHMARK
    void paintTime( qint64 );
    void displayListTime( qint64 );
    void paintingDone();
#endif // BENCHMARK


public slots:
    void addTimeSeries();
    void addEventEditor();
    void addIntervalEditor();
    void registerCompleted ( GL_Layer* module );
    void ExtLayerUpdate ( GL_Layer* mod );
    void updateCoords ( long double Xcursor, double Xscale, double Ymin, double Ymax );
    void displaySpot ( long double Xcursor, double Xscale, double Ymin, double Ymax );
    void displayRange ( long double begin, long double end );
};

}

#endif
// kate: indent-mode cstyle; space-indent on; indent-width 0; 
