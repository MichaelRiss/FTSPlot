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


#ifndef EVENTEDITOR_H
#define EVENTEDITOR_H

#include <GL_Layer.h>
#include <QBitmap>
#include "ui_EventEditorGUI.h"
#include "EventEditorLoader.h"
#include "GUIstate.h"
#include "HideNotifyWidget.h"

namespace FTSPlot
{

class EventEditor : public GL_Layer
{
    Q_OBJECT
public:
    EventEditor ( SimpleViewWidget* plotWidget );
    ~EventEditor();
    void paintGL();
    void genDisplayList ( qint64 reqXdataBegin, qint64 reqXdataEnd, int reqDispPower, double reqYFrustMax, double reqYFrustMin );
    void toggleLists();
    double getMin();
    double getMax();
    void mouseMoveEvent ( QMouseEvent* event );
    void mousePressEvent ( QMouseEvent* event );
    void wheelEvent ( QWheelEvent* event );
    void keyPressEvent ( QKeyEvent* event );
    void keyReleaseEvent ( QKeyEvent* event );
    void setColor ( QColor color );
    QString getPath();
    void showGUI();
    void hideGUI();
    bool hasGUI();
    bool GUIvisible();
    bool openTreeDir ( QString EventListDirectory );
    bool closeTreeDir();
    bool addEvent ( quint64 xpos );
    bool hasEvent ( quint64 xpos );
    bool delEvent ( quint64 event );
    quint64 getCurrentEvent();
    bool setCurrentEvent ( quint64 xpos );
    bool importFlatFile ( QString flatFileName );
    bool exportFlatFile ( QString flatFileName );
    bool hasNextEvent( quint64 event );
    bool hasPrevEvent( quint64 event );
    quint64 nextEvent( quint64 event ) throw ( bool );
    quint64 prevEvent( quint64 event ) throw ( bool );
    qint64 getXmin();
    qint64 getXmax();

private:
    Ui::EventEditorGUI ui;
    HideNotifyWidget* gui;
    QString eventListDirName;
    QString treeRootDirName;
    QString flatFileName;
    QString originalModuleName;
    quint64 currentEvent;
    GUIstate state;
    void updateGUI();
    bool RemoveDirectory ( QDir );
    QString generatePath( quint64 event, int height, QString suffix = "");
    bool generateNodeFiles ( QString baseDirName, QStringList filePath, QStringList oldPath );
    bool recursiveTreeExport ( QString currentDirName, QFile& flatFile, int height ) throw (QString);
    SimpleViewWidget* svw;
    QCursor DeleteCursor;
    QList<qint64> eventLists[2];
    EventEditorLoader* eel;
    QThread* workerThread;
    bool delEventInRange ( quint64 xBegin, quint64 xEnd );
    bool hasEvent ( quint64 xBegin, quint64 xEnd, QString path, quint64 pathValue, int height );
    bool findFirstEvent ( quint64* event, QString path, int height );
    bool findLastEvent ( quint64* event, QString path, int height );
    bool isFirstEvent ( quint64 ev );
    bool isLastEvent ( quint64 ev );
    bool delEvent_sub ( quint64 xBegin, quint64 xEnd, quint64* deletedEvent, QString path, quint64 pathValue, int height ); 
    QString manualPosValue;
    QColor myColor;
    GLfloat red;
    GLfloat green;
    GLfloat blue;
    qint64 XdataBegin;
    qint64 reqXdataBegin;
    int displayPower;
    int reqDisplayPower;
    double ymin;
    double ymax;
    void updateCursor();
    bool searchNextEvent ( bool firstDescent, quint64 event, quint64* nextEvent, QString path, int height );
    bool searchPrevEvent ( bool firstDescent, quint64 event, quint64* prevEvent, QString path, int height );
    bool isEmpty();
    QString fineEditValue;
    bool importFlatFileMain( QString flatFileName ) throw (QString);
    bool exportFlatFileMain( QString flatFileName ) throw (QString);
    void fullCacheRebuild ( QString path, int height );
    void partialCacheRebuild ( QStringList pathToBlockFile );
    quint64 value2BlockBegin( quint64 value, int height = 0 );
    quint64 value2BlockEnd( quint64 value, int height = 0 );
    bool isOverEvent();
    
public slots:
    void openEventList();
    void closeEventList();
    void importFlatFileSlot();
    void exportFlatFileSlot();
    void threadDone();
    void addEventManual();
    void validateManualPosLine ( QString newValue );
    void nextEventSlot();
    void prevEventSlot();
    void deleteEvent();
    void plusEvent();
    void minusEvent();
    void handleFineTune();
    void toggleTrackBox ( int state );
    void relayGUIupdate();
signals:
    void requestDisplayList ( qint64 reqXdataBegin, qint64 reqXdataEnd,
                              int reqDispPower, QString baseDirName,
                              double reqYFrustMin, double reqYFrustMax );
    void centerCoordsOn ( long double Xcursor, double Xscale, double Ymin, double Ymax );
};

}

#endif // EVENTEDITOR_H
// kate: indent-mode cstyle; space-indent on; indent-width 4; 

