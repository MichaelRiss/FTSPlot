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


#ifndef YSCALEBAR_H
#define YSCALEBAR_H

#include <iostream>
#include <QGLWidget>
#if defined(Q_WS_MAC)
# include <OpenGL/glu.h>
#else
# ifndef QT_LINUXBASE
#  include <GL/glu.h>
# endif
#endif
#include <QDebug>

using namespace std;

namespace FTSPlot
{

class YScaleBar : public QGLWidget
{
Q_OBJECT
public:
    YScaleBar( QWidget* parent = 0 );

protected:
    void initializeGL();
    void paintGL();
    void resizeGL ( int width, int height );

private:
    int width;
    int height;
    int minDeltaticks;
    int data2screen( double dataCoord );
    double Ymin;
    double Ymax;

    QFont fixedFont;
    QString qtlabelbuffer;

public slots:
    void updateCoords( double Ymin, double Ymax );
};

}

#endif // YSCALEBAR_H
