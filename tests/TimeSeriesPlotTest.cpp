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


#include "TimeSeriesPlotTest.h"

void TimeSeriesPlotTest::initTestCase()
{
    tsplot = new FTSPlotWidget( NULL );
    QVERIFY( tsplot != NULL );
    tsplot->show();
    
    QDir testDir( "testData" );
    QVERIFY( testDir.exists() == true );
    QFile testFile( "testData/timeSeriesDataFile.bin.cfg" );
    QVERIFY( testFile.exists() == true );
}

void TimeSeriesPlotTest::cleanupTestCase()
{
    tsplot->hide();
    delete tsplot;
}

void TimeSeriesPlotTest::createAndDestroyTimeSeriesModule()
{
    tsModule = tsplot->addTimeSeries( "testData/timeSeriesDataFile.bin.cfg" );
    QVERIFY( tsModule != NULL );
    bool result = tsplot->deleteModule( tsModule );
    QVERIFY( result == true );
}


#include "TimeSeriesPlotTest.moc"
