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


#ifndef FTSPLOTBENCHMAINWINDOW_H
#define FTSPLOTBENCHMAINWINDOW_H

#include <QMainWindow>
#include <QTextStream>
#include <QFile>
#include "ui_FTSPlotBenchMainWindow.h"
#include "SimpleViewWidget.h"
#include "TimeSeriesPlot.h"
#include "EventEditor.h"
#include "IntervalEditor.h"

using namespace FTSPlot;

class FTSPlotBenchMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    FTSPlotBenchMainWindow(QWidget* parent = 0, Qt::WindowFlags flags = 0);
private:
    Ui::MainWindow ui;
    SimpleViewWidget* sView;
    TimeSeriesPlot* tsModule;
    EventEditor* EventModule;
    IntervalEditor* IntervalModule;
    qint64 benchLength;
    quint64 currentLength;
    qint32 repetition;
    bool displayListTest;
    bool displayListReset;
    QString purePaintFileName;
    QString paintFileName;
    QString prepFileName;
    QString OutputFilePrefix;
    QFile purePaintResultsFile;
    QTextStream purePaintResStream;
    QFile paintResultsFile;
    QTextStream paintResStream;
    QFile displayListResultsFile;
    QTextStream displayListResStream;
    qint64 paintTimeCache;
    qint64 displayTimeCache;
   
#ifdef COUNT_TILES
    QFile vertexCountFile;
    QTextStream vertexCountStream;
    QFile lineCountFile;
    QTextStream lineCountStream;
    QFile quadCountFile;
    QTextStream quadCountStream;
#endif // COUNT_TILES

public slots:
    void tsDatasetButtonHandler();
    void tsOutputButtonHandler();
    void EventDatasetButtonHandler();
    void EventOutputButtonHandler();
    void IntervalDatasetButtonHandler();
    void IntervalOutputButtonHandler();

    void checktsStartButton();
    void checkEventStartButton();
    void checkIntervalStartButton();

    void StarttsBenchmark();
    void StartEventBenchmark();
    void StartIntervalBenchmark();

    void handlePaintTime( qint64 );
    void handlePrepTime( qint64 );
    void paintDoneSlot();
};

#endif // FTSPLOTBENCHMAINWINDOW_H
// kate: indent-mode cstyle; space-indent on; indent-width 0; 
