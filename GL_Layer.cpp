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


#include <cmath>
#include <iostream>
#include "commonDefs.h"
#include "GL_Layer.h"

using namespace std;
using namespace FTSPlot;

GL_Layer::GL_Layer()
{
    visible = true;
}

GL_Layer::~GL_Layer()
{

}


void GL_Layer::paintGL()
{
    cout << "GL_Layer::paintGL() called" << endl;
}

void GL_Layer::genDisplayList ( qint64 reqXdataBegin, qint64 reqXdataEnd, int reqDispPower, double YFrustMin, double YFrustMax )
{
}

void GL_Layer::toggleLists()
{
}

double GL_Layer::getMin()
{
    return DoubleInfinity;
}

double GL_Layer::getMax()
{
    return -DoubleInfinity;
}

void GL_Layer::setColor ( QColor color )
{
}

QString GL_Layer::getPath()
{
    return QString();
}


void GL_Layer::mouseMoveEvent ( QMouseEvent * event )
{
    event->ignore();
}

void GL_Layer::mousePressEvent ( QMouseEvent * event )
{
    event->ignore();
}

void GL_Layer::wheelEvent ( QWheelEvent * event )
{
    event->ignore();
}

void GL_Layer::setVisible ( bool visible )
{
    this->visible = visible;
}

bool GL_Layer::isVisible()
{
    return visible;
}

void GL_Layer::enterEvent ( QEvent* )
{
}

void GL_Layer::leaveEvent ( QEvent* )
{
}

void GL_Layer::keyPressEvent ( QKeyEvent * )
{
}

void GL_Layer::keyReleaseEvent ( QKeyEvent * )
{
}

qint64 GL_Layer::getXmin()
{
    return -1;
}

qint64 GL_Layer::getXmax()
{
    return -1;
}

void GL_Layer::showGUI()
{
}

void GL_Layer::hideGUI()
{
}

bool GL_Layer::hasGUI()
{
    return false;
}

bool GL_Layer::GUIvisible()
{
    return false;
}

