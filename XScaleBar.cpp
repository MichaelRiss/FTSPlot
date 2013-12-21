/* O1Plot - fast time series dataset plotter
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
#include "XScaleBar.h"

#define MAX(a,b) a<b?b:a

using namespace O1Plot;

XScaleBar::XScaleBar( QWidget* parent )
        : QGLWidget( parent )
{
    qtminDeltaticks = 3;
    Xcursor = 0;
    Xscale = 1;
    Xmax = 1;

    fixedFont.setFamily( "fixed" );
    fixedFont.setPointSize( 8 );
    fixedFont.setStyleStrategy( QFont::OpenGLCompatible );
    fixedFont.setStyleHint( QFont::TypeWriter );
    fm = new QFontMetrics( fixedFont );
}

void XScaleBar::initializeGL()
{
    glClearColor(1.0, 1.0, 1.0, 0.0);
    glShadeModel(GL_FLAT);
    glEnable( GL_LINE_SMOOTH );
    glEnable( GL_BLEND );
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
    glLineWidth(1.0);
    glDisable(GL_DEPTH_TEST);
}

void XScaleBar::resizeGL ( int width, int height )
{
    this->width = width;
    this->height = height;
    glViewport(0, 0, (GLint)width, (GLint)height);
    glMatrixMode ( GL_PROJECTION );
    glLoadIdentity ();
    gluOrtho2D( 0.0, (double) width, (double) height, 0 );
}

void XScaleBar::paintGL()
{
    glClear ( GL_COLOR_BUFFER_BIT );

    glColor3f ( 0.0, 0.0, 0.0 );
    glLineWidth( 2.0 );
    glBegin ( GL_LINES );
    glVertex2d ( 0, 0 );
    glVertex2d ( width, 0 );
    glEnd();
    glLineWidth( 1.0 );

    // Draw tick marks
    long double XDelta = Xscale * (double) width;
    long double XMax = Xcursor + XDelta;
    qtlabelbuffer = " " + QString::number( XMax, 'f', 0 );
    int labelWidth = fm->width( qtlabelbuffer );
    long double powerDueToLabel = ceill( log10l( XDelta / ( (long double) width / labelWidth ))) - 1;
    long double power = MAX( 0, ceill( log10l( XDelta / ( (long double)width / (long double)qtminDeltaticks ) ) ) );
    power = MAX( power, powerDueToLabel );
    long double XStart =  powl( 10, power ) * ceill( Xcursor / powl(10, power) );
    long double XEnd = powl( 10, power ) * floorl( screen2data(width) / powl(10, power) );

    glBegin ( GL_LINES );
    for ( long double idx = XStart; idx <= XEnd; idx += powl(10, power) )
    {
        glVertex2d( data2screen(idx), 0 );
        glVertex2d( data2screen(idx), 5 );
    }

    XStart =  powl( 10, power+1 ) * ceill( Xcursor / powl(10, power+1) ) - powl(10, power+1)/2.0 ;
    if ( XStart < Xcursor )
    {
        XStart += powl(10, power+1);
    }
    XEnd = powl( 10, power+1 ) * floorl( screen2data(width) / powl(10, power+1) );
    glBegin ( GL_LINES );
    for ( long double idx = XStart; idx <= XEnd; idx += powl(10, power+1) )
    {
        glVertex2d( data2screen(idx), 0 );
        glVertex2d( data2screen(idx), 8 );
    }

    XStart =  powl( 10, power+1 ) * ceill( Xcursor / powl(10, power+1) );
    XEnd = powl( 10, power+1 ) * floorl( screen2data(width) / powl(10, power+1) );
    for ( long double idx = XStart; idx <= XEnd; idx += powl(10, power+1) )
    {
        glVertex2d( data2screen(idx), 0 );
        glVertex2d( data2screen(idx), 10 );
    }

    glEnd();


    for ( long double idx = XStart; idx <= XEnd; idx += powl(10, power+1) )
    {
        qtlabelbuffer = QString::number( idx, 'f', 0 );
        renderText( data2screen(idx) - fm->width( qtlabelbuffer ) / 2, 20, qtlabelbuffer, fixedFont );
    }

    glFlush();
}

void XScaleBar::updateCoords( long double Xcursor, double Xscale )
{
    this->Xcursor = Xcursor;
    this->Xscale = Xscale;
    this->makeCurrent();
    update();
}

long double XScaleBar::screen2data( int screenCoord)
{
    return Xcursor + (long double) screenCoord * (long double) Xscale;
}

int XScaleBar::data2screen( long double dataCoord )
{
    return (int) ( ( (long double) dataCoord - Xcursor) / (long double) Xscale );
}

void XScaleBar::setNewXmin(long double Xmin)
{
    this->Xmin = Xmin;
}

void XScaleBar::setNewXmax( long double Xmax )
{
    this->Xmax = Xmax;

    // Cached Calculations
    stringstream buffer;
    buffer << Xmax;

    QString label;
    label = QString::number( Xmax, 'f', 0 );
    int qtlabelwidth = fm->width(label);
    qtminDeltaticks = MAX( 3, ceill( qtlabelwidth / 10.0 ) );

    update();
}

// kate: indent-mode cstyle; space-indent on; indent-width 0; 
