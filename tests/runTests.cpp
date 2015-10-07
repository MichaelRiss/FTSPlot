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


#include <QtTest/QTest>
#include "SimpleViewTest.h"
#include "TimeSeriesPlotTest.h"
#include "EventEditorTest.h"
#include "IntervalEditorTest.h"
#include "ListModulesTest.h"
#include "ShowHideTest.h"
#include "ModifyColorTest.h"
#include "EditModuleTest.h"
#include "ReorderModulesTest.h"
#include "XRangeTest.h"
#include "YRangeTest.h"
#include "EventsTest.h"
#include "IntervalsTest.h"

#include "FTSPrep.h"
#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

bool RemoveDirectory ( QDir aDir )
{
    bool has_err = false;
    if ( aDir.exists() )
    {
        QFileInfoList entries = aDir.entryInfoList ( QDir::NoDotAndDotDot |
                                QDir::Dirs | QDir::Files );
        int count = entries.size();
        for ( int idx = 0; idx < count; idx++ )
        {
            QFileInfo entryInfo = entries[idx];
            QString path = entryInfo.absoluteFilePath();
            if ( entryInfo.isDir() )
            {
                has_err = RemoveDirectory ( QDir ( path ) );
            }
            else
            {
                QFile file ( path );
                if ( !file.remove() )
                    has_err = true;
            }
        }
        if ( !aDir.rmdir ( aDir.absolutePath() ) )
            has_err = true;
    }
    return ( has_err );
}

int main( int argc, char** argv )
{
  
#ifdef Q_WS_X11
    XInitThreads();
#endif
  
    QApplication a(argc, argv);

    // delete test data directory
    QDir delDir( "testData" );
    RemoveDirectory( delDir );

    // Create directory for test data
    {
        QDir currentDir( "." );
        if ( !currentDir.mkdir( "testData" ) )
        {
            qDebug() << "Cannot create directory \"testData\".";
            return -1;
        }
    }

    // create some time series test data
    {
        QFile timeSeriesDataFile( "testData/timeSeriesDataFile.bin" );
        if ( !timeSeriesDataFile.open( QIODevice::WriteOnly ) )
        {
            qDebug() << "Cannot create file: \"testData/timeSeriesDataFile.bin\".";
            return -1;
        }
        qint64 periods = 10;
        for ( qint64 i = 0; i < periods; i++ )
        {
            QVector<double> buffer;
            qint64 length = (quint64) 2 << i;
            for ( qint64 j = 0; j < length; j++ )
            {
                buffer.append( sin( 2 * M_PI * (double)j / (double) length ) );
            }
            if ( timeSeriesDataFile.write( (const char*) buffer.data(), sizeof(double) * length ) != sizeof(double) * length )
            {
                qDebug() << "Cannot write to file: \"testData/timeSeriesDataFile.bin\".";
                return -1;
            }
        }
        timeSeriesDataFile.close();
    }

    // "thin" test data
    {
      FTSPrep dataPrep;
      dataPrep.setNewFile( "testData/timeSeriesDataFile.bin" );
      dataPrep.start();
      dataPrep.wait();
    }
    
    // Create directory for event editor test data
    {
        QDir currentDir( "." );
        if ( !currentDir.mkdir( "testData/EventEditorTest" ) )
        {
            qDebug() << "Cannot create directory \"testData/EventEditorTest\".";
            return -1;
        }
    }
    
    // Create directory for interval editor test data
    {
        QDir currentDir( "." );
        if ( !currentDir.mkdir( "testData/IntervalEditorTest" ) )
        {
            qDebug() << "Cannot create directory \"testData/IntervalEditorTest\".";
            return -1;
        }
    }


    SimpleViewTest svt;
    QTest::qExec( &svt );
    
    TimeSeriesPlotTest tspt;
    QTest::qExec( &tspt );
    
    EventEditorTest eet;
    QTest::qExec( &eet );

    IntervalEditorTest iet;
    QTest::qExec( &iet );

    ListModulesTest lmt;
    QTest::qExec( &lmt );
    
    ShowHideTest sht;
    QTest::qExec( &sht );
    
    ModifyColorTest mct;
    QTest::qExec( &mct );
    
    EditModuleTest emt;
    QTest::qExec( &emt );
    
    ReorderModulesTest rot;
    QTest::qExec( &rot );
    
    XRangeTest xrt;
    QTest::qExec( &xrt );
    
    YRangeTest yrt;
    QTest::qExec( &yrt );
    
    EventsTest evt;
    QTest::qExec( &evt );
    
    IntervalsTest ivt;
    QTest::qExec( &ivt );
    
    return 0;
}
