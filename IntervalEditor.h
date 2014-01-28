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


#ifndef INTERVALEDITOR_H
#define INTERVALEDITOR_H

#include <GL_Layer.h>
#include "ui_IntervalEditorGUI.h"
#include "IntervalListLoader.h"
#include "SimpleViewWidget.h"
#include "GUIstate.h"
#include "Interval.h"
#include "HideNotifyWidget.h"

namespace FTSPlot
{

class IntervalEditor : public GL_Layer
{
    Q_OBJECT
public:
    IntervalEditor ( SimpleViewWidget* plotWidget );
    ~IntervalEditor();
    void paintGL();
    void genDisplayList ( qint64 reqXdataBeginArg, qint64 reqXdataEndArg, int reqDispPowerArg, double reqYFrustMin, double reqYFrustMax );
    void toggleLists();
    double getMin();
    double getMax();
    void mouseMoveEvent ( QMouseEvent* eventArg );
    void mousePressEvent ( QMouseEvent* eventArg );
    void wheelEvent ( QWheelEvent* eventArg );
    void keyPressEvent ( QKeyEvent* eventArg );
    void keyReleaseEvent ( QKeyEvent* eventArg );
    void setColor ( QColor color );
    QString getPath();
    void showGUI();
    void hideGUI();
    bool hasGUI();
    bool GUIvisible();
    bool openTreeDir ( QString TreeDirPath );
    bool closeTreeDir();
    bool addInterval ( Interval inter );
    bool hasInterval ( Interval inter );
    bool delInterval ( Interval inter );
    Interval getCurrentInterval();
    bool setCurrentInterval ( Interval inter );
    bool importFlatFile ( QString flatFileName );
    bool exportFlatFile ( QString flatFileName );
    bool hasNextInterval ( Interval inter );
    bool hasPrevInterval ( Interval inter );
    Interval nextInterval ( Interval inter ) throw ( bool );
    Interval prevInterval ( Interval inter ) throw ( bool );
    qint64 getXmin();
    qint64 getXmax();

private:
    Ui::IntervalEditorGUI ui;
    HideNotifyWidget* gui;
    QString intervalListDirName;
    QString treeRootDirName;
    QString originalModuleName;
    GUIstate state;
    bool findFirstInterval ( FTSPlot::Interval& inter );
    bool findLastInterval( Interval& inter );
    QVector< Interval > findFirstInterval ( QString path );
    QVector< Interval > findLastInterval( QString path );
    int sliceWidth;
    Interval currentInterval;
    void updateGUI();
    IntervalListLoader* ill;
    SimpleViewWidget* svw;
    bool RemoveDirectory ( QDir aDir );
    QString flatFileName;
    bool recursiveTreeExport ( QVector< FTSPlot::Interval >& blockArray, QString currentDirName, QFile& flatFile ) throw (QString);
    int reqDisplayPower;
    double ymin;
    double ymax;
    qint64 reqXdataBegin;
    QColor myColor;
    GLfloat red;
    GLfloat green;
    GLfloat blue;
    qint64 XdataBegin;
    int displayPower;
    qint64 firstPoint;
    qint64 currentPoint;
    bool firstPointSet;
    QCursor DeleteCursor;
    void partialNodeRebuild ( QStringList pathToBlockFile );
    bool findInterval ( Interval& inter, qint64 xBegin, qint64 xEnd );
    bool findInterval_sub ( Interval& inter, qint64 xBegin, qint64 xEnd, QString path, quint64 pathValue, int height );
    void cleanupDirectories ( QStringList pathList );
    bool isEmpty();
    bool searchNextInterval ( Interval inter, Interval& nextInter );
    //bool searchNextInterval_sub ( FTSPlot::Interval inter, FTSPlot::Interval& nextInter, bool firstDescent, QString path, QVector< FTSPlot::Interval > vec, int height );
    bool searchNextInterval_sub ( FTSPlot::Interval inter, FTSPlot::Interval& nextInter, bool firstDescent, QString path, int height );
    bool searchPrevInterval ( Interval inter, Interval& prevInter );
    QVector< Interval > searchPrevInterval_sub ( FTSPlot::Interval inter, bool firstDescent, QString path, int height );
    QStringList orderBlockBeforeDir ( QStringList in );
    QStringList orderDirBeforeBlock ( QStringList in );
    QString manualLow;
    QString manualHigh;
    QString fineTuneLow;
    QString fineTuneHigh;
    void trackUpdate();
    bool importFlatFileMain ( QString flatFileName ) throw ( QString );
    bool exportFlatFileMain ( QString flatFileName ) throw ( QString );
    QString generatePath( Interval inter, int height, QString suffix = "");
    QString genBlockFileName( Interval inter );
    int interval2height( Interval inter );
    bool isOverInterval();

public slots:
    void openIntervalList();
    void closeIntervalList();
    void importFlatFileSlot();
    void exportFlatFileSlot();
    void threadDone();
    void nextIntervalSlot();
    void prevIntervalSlot();
    void manualAddInterval();
    void validateManualLow();
    void validateManualHigh();
    void manualDelInterval();
    void lowerMinusHandler();
    void lowerPlusHandler();
    void upperMinusHandler();
    void upperPlusHandler();
    void validateFineTuneHigh();
    void validateFineTuneLow();
    void handleFineTuneLow();
    void handleFineTuneHigh();
    void toggleTrackBox ( int state );
    void toggleFitBox ( int state );
    void relayGUIupdate();

signals:
    void requestDisplayList ( qint64 reqXdataBegin, qint64 reqXdataEnd,
                              int reqDispPower, QString treeDir,
                              double reqYFrustMin, double reqYFrustMax );
    void centerOn ( long double Xcursor, double Xscale, double Ymin, double Ymax );
    void fitOn ( long double begin, long double end );
};

}

#endif // INTERVALEDITOR_H
// kate: indent-mode cstyle; space-indent on; indent-width 4; 
