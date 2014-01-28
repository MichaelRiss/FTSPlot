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


#include <QDebug>
#include "YRangeTest.h"

void YRangeTest::initTestCase()
{
    tsplot = new SimpleViewWidget ( NULL );
    QVERIFY ( tsplot != NULL );
    tsplot->show();
}

void YRangeTest::cleanupTestCase()
{
    bool result;
    result = tsplot->deleteModule ( tsModule );
    QVERIFY ( result == true );

    delete tsplot;
}

void YRangeTest::YRangeTesting()
{
    tsplot->setYRange ( -0.76, 1.37 );
    QVERIFY( tsplot->getYRange() == ( QPair<double, double>( -0.76, 1.37 ) ) );
  
    QDir testDir ( "testData" );
    QVERIFY ( testDir.exists() == true );

    QFile testFile ( "testData/timeSeriesDataFile.bin.cfg" );
    QVERIFY ( testFile.exists() == true );
    tsModule = tsplot->addTimeSeries ( "testData/timeSeriesDataFile.bin.cfg" );
    QVERIFY ( tsModule != NULL );
    
    QVERIFY( tsplot->getYRange() != ( QPair<double, double>( -0.76, 1.37 ) ) );
    tsplot->setYRange ( -1.09, 0.45 );
    QVERIFY( tsplot->getYRange() == ( QPair<double, double>( -1.09, 0.45 ) ) );
}

#include "YRangeTest.moc"
