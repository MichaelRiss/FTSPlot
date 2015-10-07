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


#include <iostream>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QTime>
#include <QDir>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <FTSPlotWidget.h>
#include <math.h>
#include "TimeSeriesPlot.h"

using namespace std;
using namespace FTSPlot;

TimeSeriesPlot::TimeSeriesPlot( FTSPlotWidget* glwindow ) :
		red(0.0), green(0.0), blue(0.0),
		workerThread( new QThread() )
{
    useList = -1;
    genList = -1;

    this->GLCanvas = glwindow;

    // reserve two displaylists
    glwindow->makeCurrent();
    displayLists[0] = glGenLists( 1 );
    displayLists[1] = glGenLists( 1 );
    if ( displayLists[0] == 0 || displayLists[1] == 0 )
    {
        qDebug() << "Error: Cannot reserve display list. Exiting.";
        exit(1);
    }
    worker = new TimeSeriesPlotLoader();
    worker->moveToThread( workerThread );
    workerThread->start();
    qRegisterMetaType<GLuint>("GLuint");
    qRegisterMetaType<displaylistdata<double>*>("DisplayListData");
    connect( this, SIGNAL(requestNewDisplayLists(qint64, qint64, int )),
    		 worker, SLOT(genDisplayList(qint64, qint64, int )) );
    connect( worker, SIGNAL( notifyListUpdate( displaylistdata<double>* ) ), this, SLOT( receiceListUpdate( displaylistdata<double>* ) ) );
}

TimeSeriesPlot::~TimeSeriesPlot()
{
    disconnect( worker, SIGNAL( notifyListUpdate( displaylistdata<double>* ) ), this, SLOT( receiceListUpdate( displaylistdata<double>* ) ) );
    workerThread->quit();
    workerThread->wait();
    delete( workerThread );
    delete( worker );

    // delete displayLists
    GLCanvas->makeCurrent();
    glDeleteLists( displayLists[1], 1 );
    glDeleteLists( displayLists[0], 1 );
}

bool TimeSeriesPlot::openFile ( QString filename )
{
    fileName = filename;
    return worker->openFile( filename );
}

double TimeSeriesPlot::getMin()
{
    return worker->getMin();
}

double TimeSeriesPlot::getMax()
{
    return worker->getMax();
}

void TimeSeriesPlot::setColor( QColor color )
{
    myColor = color;
    red = myColor.redF();
    green = myColor.greenF();
    blue = myColor.blueF();
}

QString TimeSeriesPlot::getPath()
{
    return fileName;
}

void TimeSeriesPlot::genDisplayList( qint64 Xbegin, qint64 Xend, int reqPower, double reqYFrustMin, double reqYFrustMax )
{
    // use right displaylist
    if ( useList == 1 || useList == -1 )
    {
        genList = 0;
    }
    else
    {
        genList = 1;
    }
    emit requestNewDisplayLists( Xbegin, Xend, reqPower );
}

void TimeSeriesPlot::toggleLists()
{
    // No need for synchronization as we are called synchronously to
    // the SimplePlotWidget paintGL() method
    useList = genList;
    genList = -1;
}

void TimeSeriesPlot::paintGL()
{
    if ( useList != -1 )
    {
        glColor3f ( red, green, blue );
        glCallList( displayLists[useList] );
    }
}

void TimeSeriesPlot::receiceListUpdate( displaylistdata<double>* dList )
{
	// check whether there is a valid GL context
	if( !GLCanvas->isValid() ){
		qDebug() << "TimeSeriesPlot::receiceListUpdate(): GLCanvas is not valid, exiting.";
		return;
	}

	// make current
	GLCanvas->makeCurrent();

	// generate displaylist
	// (hand over a pointer via argument to struct with Type + QVector?)
	glNewList ( displayLists[genList], GL_COMPILE );
	glBegin ( dList->drawtype );
	Q_ASSERT( dList->maxIdx % 2 == 0 );
	for( int i = 0; i < dList->maxIdx/2; i++ ){
		glVertex2d( dList->data[2*i], dList->data[2*i+1] );
	}
	glEnd();
	glEndList();

    emit notifyListUpdate( this );
}

qint64 TimeSeriesPlot::getXmin() {
    return worker->getXMin();
}

qint64 TimeSeriesPlot::getXmax() {
    return worker->getXMax();
}

