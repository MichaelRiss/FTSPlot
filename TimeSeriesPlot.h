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


#ifndef __TIMESERIESPLOT_H__
#define __TIMESERIESPLOT_H__

#include "SimpleViewWidget.h"
#include "GL_Layer.h"
#include "mmapFileInfo.h"
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QGLWidget>
#include "TimeSeriesPlotLoader.h"

namespace FTSPlot
{

class TimeSeriesPlot : public GL_Layer
{
    Q_OBJECT
public:
    TimeSeriesPlot( SimpleViewWidget* glwindow );
    ~TimeSeriesPlot();
    bool openFile( QString filename );
    double getMin();
    double getMax();
    void genDisplayList( qint64 Xbegin, qint64 Xend, int reqPower, double reqYFrustMin, double reqYFrustMax );
    void toggleLists();
    void paintGL();
    void setColor( QColor color );
    QString getPath();
    qint64 getXmin();
    qint64 getXmax();

private:
    GLuint displayLists[2];
    int useList;
    int genList;
    SimpleViewWidget* glwindow;
    TimeSeriesPlotLoader* worker;
    QColor myColor;
    GLfloat red;
    GLfloat green;
    GLfloat blue;
    QString fileName;
public slots:
    void receiceListUpdate();
signals:
    //void notifyListUpdate( GL_Layer* module );
    void triggerRepaint();
};

}

#endif // __TIMESERIESPLOT_H__
