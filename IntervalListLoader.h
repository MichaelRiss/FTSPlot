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


#ifndef INTERVALLISTLOADER_H
#define INTERVALLISTLOADER_H

#include <QThread>
#include <QGLWidget>
#include <QMutex>
#include <QWaitCondition>
#include "Interval.h"
#include "EditorUtils.hpp"

namespace O1Plot
{

class IntervalListLoader : public QThread
{
    Q_OBJECT
public:
    IntervalListLoader ( QGLWidget* parent = 0 );
    ~IntervalListLoader();
    void paintGL();
    void eventLoopAlive();
    void setColor( QColor color );
    void toggleLists();

protected:
    virtual void run();

private:
    QMutex lock;
    QWaitCondition waitCond;
    int eventLoopTestCounter0;
    int eventLoopTestCounter1;
    QGLWidget* glwidget;
    QGLWidget* myGLwidget;
    //QGLContext* myGLContext;
    GLuint displayLists[2];
    int useList;
    int genList;
    QColor myColor;
    GLfloat red;
    GLfloat green;
    GLfloat blue;
    void getRecursiveEvents( InlineVec<GLdouble>& BoxArray, InlineVec<GLdouble>& LineArray, quint64 beginIdx, quint64 endIdx, QString path, quint64 pathValue, int reqDispPower, qint64 reqXdataBegin, double ymin, double ymax, int height );
    
signals:
    void notifyListUpdate();
    void checkEventLoopSignal ( int counter );

public slots:
    void genDisplayList ( qint64 reqXdataBegin, qint64 reqXdataEnd, int reqDispPower, QString treeDirName, double ymin, double ymax );
    void checkEventLoop ( int counter );

};

class IntervalListLoader_Suspend
{
public:
    IntervalListLoader_Suspend ( IntervalListLoader* lockarg );
    ~IntervalListLoader_Suspend();

private:
    IntervalListLoader* lock;
};

}

#endif // INTERVALLISTLOADER_H
// kate: indent-mode cstyle; space-indent on; indent-width 4; 
