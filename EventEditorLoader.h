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


#ifndef EVENTLISTLOADER_H
#define EVENTLISTLOADER_H

#include <DisplayListData.h>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include "EditorUtils.hpp"


namespace FTSPlot
{

class EventEditorLoader : public QObject
{
    Q_OBJECT
public:
    EventEditorLoader ();
    ~EventEditorLoader();
    displaylistdata<double> dList;

private:
    void getRecursiveEvents( quint64 beginIdx, quint64 endIdx, QString path, quint64 pathValue, int height, int reqDispPower, qint64 reqXdataBegin, double ymin, double ymax );

#ifdef COUNT_TILES
    qint64 vertexCount;
    qint64 lineCount;
#endif // COUNT_TILES
    
public slots:
    void genDisplayList ( qint64 reqXdataBegin, qint64 reqXdataEnd, int reqDispPower, QString baseDirName, double ymin, double ymax );

signals:
	void notifyListUpdate( displaylistdata<double>* dList );
	void checkEventLoopSignal( int counter );
};

class EventEditorLoader_Suspend
{
public:
    EventEditorLoader_Suspend( QThread* thread );
    ~EventEditorLoader_Suspend();
    
private:
    QThread* lock;
};

}

#endif // EVENTLISTLOADER_H

