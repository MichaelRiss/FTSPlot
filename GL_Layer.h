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


#ifndef __GL_LAYER_H__
#define __GL_LAYER_H__

namespace O1Plot
{

class GL_Layer;
}

#include <QObject>
#include <QMouseEvent>
#include "SimpleViewWidget.h"

namespace O1Plot
{

class GL_Layer : public QObject
{
    Q_OBJECT
public:
    GL_Layer();
    virtual ~GL_Layer();
    virtual void paintGL();
    virtual void genDisplayList( qint64 reqXdataBegin, qint64 reqXdataEnd, int reqDispPower, double YFrustMin, double YFrustMax );
    virtual void toggleLists();
    virtual double getMin();
    virtual double getMax();
    virtual void mouseMoveEvent ( QMouseEvent * event );
    virtual void mousePressEvent ( QMouseEvent * event );
    virtual void wheelEvent ( QWheelEvent * event );
    virtual void enterEvent ( QEvent* event );
    virtual void leaveEvent ( QEvent* event );
    virtual void keyPressEvent ( QKeyEvent * event );
    virtual void keyReleaseEvent ( QKeyEvent * event );
    virtual void setColor( QColor color );
    virtual void setVisible( bool visible );
    virtual bool isVisible();
    virtual QString getPath();
    virtual qint64 getXmin();
    virtual qint64 getXmax();
    virtual void showGUI();
    virtual void hideGUI();
    virtual bool hasGUI();
    virtual bool GUIvisible();

private:
  bool visible;
    
signals:
    void notifyListUpdate( GL_Layer* );
    void triggerRepaint( GL_Layer* );
    void GUIshowhide();
#ifdef BENCHMARK
    void paintTime( int );
#endif // BENCHMARK
};

}

#endif // __GL_LAYER_H__
