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


#ifndef __TIMESERIESPLOTLOADER_H__
#define __TIMESERIESPLOTLOADER_H__

#include <cmath>
#include <iostream>
#include <QThread>
#include <QGLWidget>
#if defined(Q_WS_MAC)
# include <OpenGL/glu.h>
#else
# ifndef QT_LINUXBASE
#  include <GL/glu.h>
# endif
#endif
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QMutex>
#include <QWaitCondition>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "mmapFileInfo.h"

namespace FTSPlot
{
    bool isPow2( int a );
    int intlog2( int a );
    
class TimeSeriesPlotLoader : public QThread
{
    Q_OBJECT
public:
    TimeSeriesPlotLoader ( QGLWidget* glwindow );
    ~TimeSeriesPlotLoader();
    bool openFile ( QString fileName );
    void genDisplayList ( qint64 Xbegin, qint64 Xend,
                          int reqPower, GLuint displayList );
    void run();
    void stop();
    double getMin();
    double getMax();
    qint64 getXMin();
    qint64 getXMax();
private:
    QGLWidget* glwindow;
    QGLWidget* myGLwidget;
    bool filesready;
    double recordMin;
    double recordMax;
    QVector <mmapFileInfo> files;
    qint64 NoSamples;
    QMutex ctrlLock;
    QWaitCondition ctrlWait;
    qint64 reqBegin;
    qint64 reqEnd;
    int reqPower;
    qint64 begin;
    qint64 end;
    int power;
    GLuint reqDispList;
    bool newJob;
    bool done;
    bool stopThread;
    GLuint dispList;
signals:
    void notifyListUpdate();
};

}

#endif
// kate: indent-mode cstyle; space-indent on; indent-width 4; 
