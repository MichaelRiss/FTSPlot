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


#include <QFileDialog>
#include <QFile>
#include <QDebug>
#include "O1PlotBenchMainWindow.h"

#if defined( COUNT_TILES ) || defined( LINUX_DISABLE_FILESYSTEMCACHE )
#include "benchmarks/benchmarkHelper.h"
#endif // COUNT_TILES

O1PlotBenchMainWindow::O1PlotBenchMainWindow ( QWidget* parent, Qt::WindowFlags flags ) : QMainWindow ( parent, flags )
{
    ui.setupUi ( this );
    ui.tsStartButton->setDisabled ( true );
    ui.EventStartButton->setDisabled ( true );
    ui.IntervalStartButton->setDisabled ( true );

    connect ( ui.tsDatasetFileButton, SIGNAL ( clicked ( bool ) ), this, SLOT ( tsDatasetButtonHandler() ) );
    connect ( ui.tsOutputButton, SIGNAL ( clicked ( bool ) ), this, SLOT ( tsOutputButtonHandler() ) );

    connect ( ui.EventDatasetFileButton, SIGNAL ( clicked ( bool ) ), this, SLOT ( EventDatasetButtonHandler() ) );
    connect ( ui.EventOutputButton, SIGNAL ( clicked ( bool ) ), this, SLOT ( EventOutputButtonHandler() ) );

    connect ( ui.IntervalDatasetFileButton, SIGNAL ( clicked ( bool ) ), this, SLOT ( IntervalDatasetButtonHandler() ) );
    connect ( ui.IntervalOutputButton, SIGNAL ( clicked ( bool ) ), this, SLOT ( IntervalOutputButtonHandler() ) );

    connect ( ui.tsDataSetLine, SIGNAL ( textChanged ( QString ) ), this, SLOT ( checktsStartButton() ) );
    connect ( ui.tsOutputLine, SIGNAL ( textChanged ( QString ) ), this, SLOT ( checktsStartButton() ) );

    connect ( ui.EventDatasetLine, SIGNAL ( textChanged ( QString ) ), this, SLOT ( checkEventStartButton() ) );
    connect ( ui.EventOutputLine, SIGNAL ( textChanged ( QString ) ), this, SLOT ( checkEventStartButton() ) );

    connect ( ui.IntervalDatasetLine, SIGNAL ( textChanged ( QString ) ), this, SLOT ( checkIntervalStartButton() ) );
    connect ( ui.IntervalOutputLine, SIGNAL ( textChanged ( QString ) ), this, SLOT ( checkIntervalStartButton() ) );

    connect ( ui.tsStartButton, SIGNAL ( clicked ( bool ) ), this, SLOT ( StarttsBenchmark() ) );
    connect ( ui.EventStartButton, SIGNAL ( clicked ( bool ) ), this, SLOT ( StartEventBenchmark() ) );
    connect ( ui.IntervalStartButton, SIGNAL ( clicked ( bool ) ), this, SLOT ( StartIntervalBenchmark() ) );
}

void O1PlotBenchMainWindow::tsDatasetButtonHandler()
{
    // show filedialog
    QString fileName = QFileDialog::getOpenFileName ( NULL, "Select TimeSeries Dataset", "", "TimeSeries Datasets (*.cfg);;All (*)" );
    // if value => copy to lineedit
    if ( fileName.size() > 0 )
    {
        ui.tsDataSetLine->setText ( fileName );
    }
}

void O1PlotBenchMainWindow::tsOutputButtonHandler()
{
    // show filedialog
    QString dirName = QFileDialog::getExistingDirectory ( NULL, "Select TimeSeries Logdirectory", "" );
    // if value => copy to lineedit
    if ( dirName.size() > 0 )
    {
        ui.tsOutputLine->setText ( dirName );
    }
}

void O1PlotBenchMainWindow::EventDatasetButtonHandler()
{
    // show directorydialog
    QString dirName = QFileDialog::getExistingDirectory ( NULL, "Select Event Dataset", "" );
    // if value => copy to lineedit
    if ( dirName.size() > 0 )
    {
        ui.EventDatasetLine->setText ( dirName );
    }
}

void O1PlotBenchMainWindow::EventOutputButtonHandler()
{
    // show filedialog
    QString dirName = QFileDialog::getExistingDirectory ( NULL, "Select Event Logdirectory", "" );
    // if value => copy to lineedit
    if ( dirName.size() > 0 )
    {
        ui.EventOutputLine->setText ( dirName );
    }
}


void O1PlotBenchMainWindow::IntervalDatasetButtonHandler()
{
    // show directorydialog
    QString dirName = QFileDialog::getExistingDirectory ( NULL, "Select Interval Dataset", "" );
    // if value => copy to lineedit
    if ( dirName.size() > 0 )
    {
        ui.IntervalDatasetLine->setText ( dirName );
    }
}

void O1PlotBenchMainWindow::IntervalOutputButtonHandler()
{
    // show filedialog
    QString dirName = QFileDialog::getExistingDirectory ( NULL, "Select Interval Logfile", "" );
    // if value => copy to lineedit
    if ( dirName.size() > 0 )
    {
        ui.IntervalOutputLine->setText ( dirName );
    }
}

void O1PlotBenchMainWindow::checktsStartButton()
{
    if ( ui.tsDataSetLine->text().size() == 0 || ui.tsOutputLine->text().size() == 0 )
    {
        ui.tsStartButton->setDisabled ( true );
    }
    if ( ui.tsDataSetLine->text().size() > 0 && ui.tsOutputLine->text().size() > 0 )
    {
        ui.tsStartButton->setEnabled ( true );
    }
}

void O1PlotBenchMainWindow::checkEventStartButton()
{
    if ( ui.EventDatasetLine->text().size() == 0 || ui.EventOutputLine->text().size() == 0 )
    {
        ui.EventStartButton->setDisabled ( true );
    }
    if ( ui.EventDatasetLine->text().size() > 0 && ui.EventOutputLine->text().size() > 0 )
    {
        ui.EventStartButton->setEnabled ( true );
    }
}

void O1PlotBenchMainWindow::checkIntervalStartButton()
{
    if ( ui.IntervalDatasetLine->text().size() == 0 || ui.IntervalOutputLine->text().size() == 0 )
    {
        ui.IntervalStartButton->setDisabled ( true );
    }
    if ( ui.IntervalDatasetLine->text().size() > 0 && ui.IntervalOutputLine->text().size() > 0 )
    {
        ui.IntervalStartButton->setEnabled ( true );
    }
}


void O1PlotBenchMainWindow::StarttsBenchmark()
{
    // disable GUI
    ui.centralwidget->setDisabled ( true );

    // create SimpleView
    sView = new SimpleViewWidget ( NULL );
    if ( sView == NULL )
    {
        qDebug() << "Cannot create SimpleViewWidget.";
        exit ( 1 );
    }
    sView->resize( 800, 600 );
    sView->show();
    // add TS
    tsModule = ( TimeSeriesPlot* ) sView->addTimeSeries ( ui.tsDataSetLine->text() );
    if ( tsModule == NULL )
    {
        qDebug() << "Cannot create TimeSeries module.";
        exit ( 1 );
    }
    connect ( sView, SIGNAL ( paintTime ( qint64 ) ), this, SLOT ( handlePaintTime ( qint64 ) ) );
    connect ( sView, SIGNAL ( displayListTime ( qint64 ) ), this, SLOT ( handlePrepTime ( qint64 ) ) );
    connect ( sView, SIGNAL ( paintingDone() ), this, SLOT ( paintDoneSlot() ) );

    // set state variables for benchmark
    displayListTest = false;

    // determine dataset length
    benchLength = tsModule->getXmax();
    currentLength = 1;
    repetition = 0;

    purePaintFileName = ui.tsOutputLine->text() + "/TimeSeriesPurePaint.";
    paintFileName = ui.tsOutputLine->text() + "/TimeSeriesPaint.";
    prepFileName =  ui.tsOutputLine->text() + "/TimeSeriesdisplayList.";

    purePaintResultsFile.setFileName ( purePaintFileName + QString::number ( currentLength ) );
    if ( !purePaintResultsFile.open ( QIODevice::WriteOnly ) )
    {
        qDebug() << "Cannot open" << purePaintFileName + QString::number ( currentLength );
        return;
    }
    purePaintResStream.setDevice ( &purePaintResultsFile );

#ifdef COUNT_TILES
    vertexCountFile.setFileName( ui.tsOutputLine->text() + "/vertexCount.txt" );
    if ( !vertexCountFile.open ( QIODevice::WriteOnly ) )
    {
        qDebug() << "Cannot open" << vertexCountFile.fileName();
        return;
    }
    vertexCountStream.setDevice( &vertexCountFile );

    lineCountFile.setFileName( ui.tsOutputLine->text() + "/lineCount.txt" );
    if ( !lineCountFile.open ( QIODevice::WriteOnly ) )
    {
        qDebug() << "Cannot open" << lineCountFile.fileName();
        return;
    }
    lineCountStream.setDevice( &lineCountFile );

    quadCountFile.setFileName( ui.tsOutputLine->text() + "/quadCount.txt" );
    if ( !quadCountFile.open ( QIODevice::WriteOnly ) )
    {
        qDebug() << "Cannot open" << quadCountFile.fileName();
        return;
    }
    quadCountStream.setDevice( &quadCountFile );

#endif // COUNT_TILES

    // kick off benchmarking
    sView->displayRange ( 0, currentLength );
}

void O1PlotBenchMainWindow::handlePaintTime ( qint64 value )
{
    paintTimeCache = value;
}

void O1PlotBenchMainWindow::handlePrepTime ( qint64 value )
{
    displayTimeCache = value;
}

void O1PlotBenchMainWindow::paintDoneSlot()
{
    if ( displayListTest == false )
    {
        purePaintResStream << paintTimeCache << endl;
        repetition++;
        if ( repetition >= BENCHREPETITIONS )
        {
            purePaintResultsFile.close();

#ifdef COUNT_TILES
            // Save statistics
            vertexCountStream << currentLength << " " << benchmarkHelper::vertexCount << endl;
            lineCountStream << currentLength << " " << benchmarkHelper::lineCount << endl;
            quadCountStream << currentLength << " " << benchmarkHelper::quadCount << endl;
            
            QString BoxListFileName = OutputFilePrefix +  "/BoxList." + QString::number( currentLength );
            QString LineListFileName = OutputFilePrefix + "/LineList." + QString::number( currentLength );
            
            QFile BoxListFile( BoxListFileName );
            QFile LineListFile( LineListFileName );
            
            if( BoxListFile.open( QIODevice::WriteOnly ) )
            {
                QTextStream BoxListStream;
                BoxListStream.setDevice( &BoxListFile );
                for( int i = 0; i < benchmarkHelper::BoxArray.getMaxIdx(); i = i + 2 )
                {
                  BoxListStream << benchmarkHelper::BoxArray[i] <<  " " << benchmarkHelper::BoxArray[i+1] << endl;
                }
            }
            
            if( LineListFile.open( QIODevice::WriteOnly ) )
            {
                QTextStream LineListStream;
                LineListStream.setDevice( &LineListFile );
                for( int i = 0; i < benchmarkHelper::LineArray.getMaxIdx(); i++ )
                {
                  LineListStream << benchmarkHelper::LineArray[i] << endl;
                }
            }
            
            LineListFile.close();
            BoxListFile.close();
#endif // COUNT_TILES            

            repetition = 0;
            if ( currentLength < Q_INT64_C(9223372036854775807) / 2 )
            {
                currentLength = currentLength * 2;
            }
            else
            {
                currentLength = 0;
            }
            if ( (currentLength != 0) && (currentLength < benchLength) )
            {
                purePaintResultsFile.setFileName ( purePaintFileName + QString::number ( currentLength ) );
                if ( !purePaintResultsFile.open ( QIODevice::WriteOnly ) )
                {
                    qDebug() << "Cannot open" << purePaintFileName + QString::number ( currentLength );
                    return;
                }
                purePaintResStream.setDevice ( &purePaintResultsFile );


#ifdef LINUX_DISABLE_FILESYSTEMCACHE
                QFile dropCacheFile( "/proc/sys/vm/drop_caches" );
                if ( !dropCacheFile.open( QIODevice::WriteOnly ) )
                {
                    qDebug() << "Cannot open /proc/sys/vm/drop_caches to flush filesystem cache.";
                }
                else
                {
                    if ( dropCacheFile.write( LINUX_DROP_CACHES_VALUE ) != 1 )
                    {
                        qDebug() << "Cannot write to /proc/sys/vm/drop_caches to flush filesystem cache.";
                    }
                    dropCacheFile.close();
                }
#endif // LINUX_DISABLE_FILESYSTEMCACHE


                sView->displayRange ( 0, currentLength );
            }
            else
            {
#ifdef COUNT_TILES
                vertexCountFile.close();
                lineCountFile.close();
                quadCountFile.close();
#endif // COUNT_TILES

                currentLength = 1;
                repetition = 0;
                displayListTest = true;
                displayListReset = false;

                paintResultsFile.setFileName ( paintFileName + QString::number ( currentLength ) );
                if ( !paintResultsFile.open ( QIODevice::WriteOnly ) )
                {
                    qDebug() << "Cannot open" << paintFileName + QString::number ( currentLength );
                    return;
                }
                paintResStream.setDevice ( &paintResultsFile );

                displayListResultsFile.setFileName ( prepFileName + QString::number ( currentLength ) );
                if ( !displayListResultsFile.open ( QIODevice::WriteOnly ) )
                {
                    qDebug() << "Cannot open" << prepFileName + QString::number ( currentLength );
                    return;
                }
                displayListResStream.setDevice ( &displayListResultsFile );
                sView->displayRange ( 0, currentLength );
            }
        }
        else
        {
            sView->update();
        }
    }
    else
    {
        if ( displayListReset )
        {
            repetition++;
            if ( repetition >= BENCHREPETITIONS )
            {
                paintResultsFile.close();
                displayListResultsFile.close();
                repetition = 0;
                if ( currentLength < Q_INT64_C(9223372036854775807) / 2 )
                {
                    currentLength = currentLength * 2;
                    qDebug() << "currentLength:" << currentLength;
                }
                else
                {
                    currentLength = 0;
                }
                if ( (currentLength != 0) && (currentLength < benchLength) )
                {
                    paintResultsFile.setFileName ( paintFileName + QString::number ( currentLength ) );
                    if ( !paintResultsFile.open ( QIODevice::WriteOnly ) )
                    {
                        qDebug() << "Cannot open" << paintFileName + QString::number ( currentLength );
                        return;
                    }
                    paintResStream.setDevice ( &paintResultsFile );

                    displayListResultsFile.setFileName ( prepFileName + QString::number ( currentLength ) );
                    if ( !displayListResultsFile.open ( QIODevice::WriteOnly ) )
                    {
                        qDebug() << "Cannot open" << prepFileName + QString::number ( currentLength );
                        return;
                    }
                    displayListResStream.setDevice ( &displayListResultsFile );

#ifdef LINUX_DISABLE_FILESYSTEMCACHE
                    QFile dropCacheFile( "/proc/sys/vm/drop_caches" );
                    if ( !dropCacheFile.open( QIODevice::WriteOnly ) )
                    {
                        qDebug() << "Cannot open /proc/sys/vm/drop_caches to flush filesystem cache.";
                    }
                    else
                    {
                        if ( dropCacheFile.write( LINUX_DROP_CACHES_VALUE ) != 1 )
                        {
                            qDebug() << "Cannot write to /proc/sys/vm/drop_caches to flush filesystem cache.";
                        }
                        dropCacheFile.close();
                    }
#endif // LINUX_DISABLE_FILESYSTEMCACHE

                    sView->displayRange ( 0, currentLength );
                    displayListReset = false;
                }
                else
                {
                    // delete simpleviewwidget
                    sView->deleteLater();
                    ui.centralwidget->setEnabled ( true );
                }
            }
            else
            {
#ifdef LINUX_DISABLE_FILESYSTEMCACHE
                if ( benchmarkHelper::filesptr != NULL )
                {
                    QString oldFile = "";
                    for ( int i = 0; i < benchmarkHelper::filesptr->size(); i++ )
                    {
                        QFile* actualFile = (*benchmarkHelper::filesptr)[i].qfile;
                        if ( actualFile->fileName() != oldFile )
                        {
                            oldFile = actualFile->fileName();
                            void* mapAddress = (*benchmarkHelper::filesptr)[i].mmap;
                            actualFile->unmap( (uchar*) mapAddress );
                            actualFile->close();
                        }
                    }
                }

                QFile dropCacheFile( "/proc/sys/vm/drop_caches" );
                if ( !dropCacheFile.open( QIODevice::WriteOnly ) )
                {
                    qDebug() << "Cannot open /proc/sys/vm/drop_caches to flush filesystem cache.";
                }
                else
                {
                    if ( dropCacheFile.write( LINUX_DROP_CACHES_VALUE ) != 1 )
                    {
                        qDebug() << "Cannot write to /proc/sys/vm/drop_caches to flush filesystem cache.";
                    }
                    dropCacheFile.close();
                }

                if ( benchmarkHelper::filesptr != NULL )
                {
                    QString oldFile = "";
                    void* mapAddress = NULL;
                    for ( int i = 0; i < benchmarkHelper::filesptr->size(); i++ )
                    {
                        QFile* actualFile = (*benchmarkHelper::filesptr)[i].qfile;
                        if ( actualFile->fileName() != oldFile )
                        {
                            oldFile = actualFile->fileName();
                            actualFile->open( QIODevice::ReadOnly );
                            mapAddress = actualFile->map( 0, actualFile->size() );
                            if ( mapAddress == NULL )
                            {
                                qDebug() << "Fatal: cannot re-map files for benchmark!";
                                exit(1);
                            }
                        }
                        (*benchmarkHelper::filesptr)[i].mmap = mapAddress;
                    }
                }
#endif // LINUX_DISABLE_FILESYSTEMCACHE

                sView->displayRange ( 0, currentLength );
                displayListReset = false;
            }
        }
        else
        {
            paintResStream << paintTimeCache << endl;
            displayListResStream << displayTimeCache << endl;
            paintTimeCache = -10;
            displayTimeCache = -10;
            sView->displayRange ( -100000, -99999 );
            displayListReset = true;
        }
    }
}


void O1PlotBenchMainWindow::StartEventBenchmark()
{
    // disable GUI
    ui.centralwidget->setDisabled ( true );

    // create SimpleView
    sView = new SimpleViewWidget ( NULL );
    if ( sView == NULL )
    {
        qDebug() << "Cannot create SimpleViewWidget.";
        exit ( 1 );
    }
    sView->resize( 800, 600 );
    sView->show();
    // add EventEditor
    EventModule = ( EventEditor* ) sView->addEventEditorModule ( ui.EventDatasetLine->text() );
    EventModule->hideGUI();
    if ( EventModule == NULL )
    {
        qDebug() << "Cannot create EventEditor module.";
        exit ( 1 );
    }
    connect ( sView, SIGNAL ( paintTime ( qint64 ) ), this, SLOT ( handlePaintTime ( qint64 ) ) );
    connect ( sView, SIGNAL ( displayListTime ( qint64 ) ), this, SLOT ( handlePrepTime ( qint64 ) ) );
    connect ( sView, SIGNAL ( paintingDone() ), this, SLOT ( paintDoneSlot() ) );

    // set state variables for benchmark
    displayListTest = false;

    // determine dataset length
    benchLength = EventModule->getXmax();
    currentLength = 1;
    repetition = 0;

    purePaintFileName = ui.EventOutputLine->text() + "/EventPurePaint.";
    paintFileName = ui.EventOutputLine->text() + "/EventPaint.";
    prepFileName =  ui.EventOutputLine->text() + "/EventdisplayList.";

    purePaintResultsFile.setFileName ( purePaintFileName + QString::number ( currentLength ) );
    if ( !purePaintResultsFile.open ( QIODevice::WriteOnly ) )
    {
        qDebug() << "Cannot open" << purePaintFileName + QString::number ( currentLength );
        return;
    }
    purePaintResStream.setDevice ( &purePaintResultsFile );

#ifdef COUNT_TILES
    vertexCountFile.setFileName( ui.EventOutputLine->text() + "/vertexCount.txt" );
    if ( !vertexCountFile.open ( QIODevice::WriteOnly ) )
    {
        qDebug() << "Cannot open" << vertexCountFile.fileName();
        return;
    }
    vertexCountStream.setDevice( &vertexCountFile );

    lineCountFile.setFileName( ui.EventOutputLine->text() + "/lineCount.txt" );
    if ( !lineCountFile.open ( QIODevice::WriteOnly ) )
    {
        qDebug() << "Cannot open" << lineCountFile.fileName();
        return;
    }
    lineCountStream.setDevice( &lineCountFile );

    quadCountFile.setFileName( ui.EventOutputLine->text() + "/quadCount.txt" );
    if ( !quadCountFile.open ( QIODevice::WriteOnly ) )
    {
        qDebug() << "Cannot open" << quadCountFile.fileName();
        return;
    }
    quadCountStream.setDevice( &quadCountFile );

#endif // COUNT_TILES

    // kick off benchmarking
    sView->displayRange ( 0, currentLength );
}

void O1PlotBenchMainWindow::StartIntervalBenchmark()
{
    // disable GUI
    ui.centralwidget->setDisabled ( true );

    // create SimpleView
    sView = new SimpleViewWidget ( NULL );
    if ( sView == NULL )
    {
        qDebug() << "Cannot create SimpleViewWidget.";
        exit ( 1 );
    }
    sView->resize( 800, 600 );
    sView->show();
    // add EventEditor
    IntervalModule = ( IntervalEditor* ) sView->addIntervalEditorModule ( ui.IntervalDatasetLine->text() );
    IntervalModule->hideGUI();
    if ( IntervalModule == NULL )
    {
        qDebug() << "Cannot create IntervalEditor module.";
        exit ( 1 );
    }
    connect ( sView, SIGNAL ( paintTime ( qint64 ) ), this, SLOT ( handlePaintTime ( qint64 ) ) );
    connect ( sView, SIGNAL ( displayListTime ( qint64 ) ), this, SLOT ( handlePrepTime ( qint64 ) ) );
    connect ( sView, SIGNAL ( paintingDone() ), this, SLOT ( paintDoneSlot() ) );

    // set state variables for benchmark
    displayListTest = false;

    // determine dataset length
    benchLength = IntervalModule->getXmin();
    qDebug() << "Min:" << benchLength;

    benchLength = IntervalModule->getXmax();
    qDebug() << "Length(Max):" << benchLength;
    currentLength = 1;
    repetition = 0;

    purePaintFileName = ui.IntervalOutputLine->text() + "/IntervalPurePaint.";
    paintFileName = ui.IntervalOutputLine->text() + "/IntervalPaint.";
    prepFileName =  ui.IntervalOutputLine->text() + "/IntervaldisplayList.";
    OutputFilePrefix = ui.IntervalOutputLine->text();

    purePaintResultsFile.setFileName ( purePaintFileName + QString::number ( currentLength ) );
    if ( !purePaintResultsFile.open ( QIODevice::WriteOnly ) )
    {
        qDebug() << "Cannot open" << purePaintFileName + QString::number ( currentLength );
        return;
    }
    purePaintResStream.setDevice ( &purePaintResultsFile );

#ifdef COUNT_TILES
    vertexCountFile.setFileName( ui.IntervalOutputLine->text() + "/vertexCount.txt" );
    if ( !vertexCountFile.open ( QIODevice::WriteOnly ) )
    {
        qDebug() << "Cannot open" << vertexCountFile.fileName();
        return;
    }
    vertexCountStream.setDevice( &vertexCountFile );

    lineCountFile.setFileName( ui.IntervalOutputLine->text() + "/lineCount.txt" );
    if ( !lineCountFile.open ( QIODevice::WriteOnly ) )
    {
        qDebug() << "Cannot open" << lineCountFile.fileName();
        return;
    }
    lineCountStream.setDevice( &lineCountFile );

    quadCountFile.setFileName( ui.IntervalOutputLine->text() + "/quadCount.txt" );
    if ( !quadCountFile.open ( QIODevice::WriteOnly ) )
    {
        qDebug() << "Cannot open" << quadCountFile.fileName();
        return;
    }
    quadCountStream.setDevice( &quadCountFile );

#endif // COUNT_TILES

    // kick off benchmarking
    sView->displayRange ( 0, currentLength );
}


#include "O1PlotBenchMainWindow.moc"
// kate: indent-mode cstyle; space-indent on; indent-width 0; 






