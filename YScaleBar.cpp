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


#include <math.h>
#include "YScaleBar.h"
#include <QPainter>

#define MAX(a,b) a<b?b:a

using namespace FTSPlot;

YScaleBar::YScaleBar(QWidget* parent)
{
    minDeltaticks = 3;

    Ymin = -1.0;
    Ymax = 1.0;

    fixedFont.setFamily( "fixed" );
    fixedFont.setPointSize( 8 );
    fixedFont.setStyleStrategy( QFont::OpenGLCompatible );
    fixedFont.setStyleHint( QFont::TypeWriter );
}

void YScaleBar::initializeGL()
{
	initializeOpenGLFunctions();
    glClearColor(1.0, 1.0, 1.0, 0.0);
    glShadeModel(GL_FLAT);
    glEnable( GL_LINE_SMOOTH );
    glEnable( GL_BLEND );
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
    glLineWidth(1.0);
    glDisable(GL_DEPTH_TEST);
}

void YScaleBar::resizeGL(int width, int height)
{
    // Looks ok
    this->width = width;
    this->height = height;
    glViewport(0, 0, (GLint)width, (GLint)height);
    glMatrixMode ( GL_PROJECTION );
    glLoadIdentity ();
    // TODO: replace with non GLU function
    gluOrtho2D( 0.0, (double) width, 0, (double) height );
}

void YScaleBar::paintGL()
{
	QPainter painter(this);
    glClear ( GL_COLOR_BUFFER_BIT );

    glColor3f ( 0.0, 0.0, 0.0 );

    glLineWidth( 2.0 );
    glBegin ( GL_LINES );
    glVertex2d ( 100, 0 );
    glVertex2d ( 100, height );
    glEnd();
    glLineWidth( 1.0 );

    // Draw tick marks
    double YDelta = Ymax - Ymin;
    double power = ceil( log10( YDelta / ( height / minDeltaticks ) ) );
    double YStart = powl( 10, power ) * floor( Ymin / powl(10, power) );
    double YEnd = powl( 10, power ) * ceil( Ymax / powl(10, power) );

    glBegin ( GL_LINES );
    for ( double idx = YStart; idx <= YEnd; idx += powl(10, power) )
    {
        glVertex2d( 95, data2screen(idx) );
        glVertex2d( 100, data2screen(idx) );
    }
    YStart =  powl( 10, power+1 ) * floor( Ymin / powl(10, power+1) );
    YEnd = powl( 10, power+1 ) * ceil( Ymax / powl(10, power+1) );
    for ( double idx = YStart; idx <= YEnd; idx += powl(10, power+1) )
    {
        glVertex2d( 90, data2screen(idx) );
        glVertex2d( 100, data2screen(idx) );
    }
    for ( double idx = YStart; idx <= YEnd; idx += powl(10, power+1)/2.0 )
    {
        glVertex2d( 92, data2screen(idx) );
        glVertex2d( 100, data2screen(idx) );
    }
    glEnd();

    // Draw labels


    for ( double idx = YStart; idx <= YEnd; idx += powl(10, power+1) )
    {
        qtlabelbuffer = QString::number( idx, 'f', 10 );
        qtlabelbuffer = QString("%1").arg(idx, 10, 'f', 7);

        painter.drawText( 10, height-data2screen(idx)+4, qtlabelbuffer );
    }

    glFlush();
}

void YScaleBar::updateCoords(double Ymin, double Ymax)
{
    this->Ymin = Ymin;
    this->Ymax = Ymax;
    update();
}

int YScaleBar::data2screen(double dataCoord) {
    return (dataCoord - Ymin) / (Ymax - Ymin) * height;
}

