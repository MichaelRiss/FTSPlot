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
#include "XRangeTest.h"

void XRangeTest::initTestCase()
{
    tsplot = new SimpleViewWidget ( NULL );
    QVERIFY ( tsplot != NULL );
    tsplot->show();
}

void XRangeTest::cleanupTestCase()
{
    bool result;
    result = tsplot->deleteModule ( tsModule );
    QVERIFY ( result == true );

    delete tsplot;
}

void XRangeTest::XRangeTesting()
{
    tsplot->setXRange ( 12.0, 130.0 );
    QVERIFY( tsplot->getXRange() == ( QPair<long double, long double>( 12.0, 130.0 ) ) );
  
    QDir testDir ( "testData" );
    QVERIFY ( testDir.exists() == true );

    QFile testFile ( "testData/timeSeriesDataFile.bin.cfg" );
    QVERIFY ( testFile.exists() == true );
    tsModule = tsplot->addTimeSeries ( "testData/timeSeriesDataFile.bin.cfg" );
    QVERIFY ( tsModule != NULL );
    
    QVERIFY( tsplot->getXRange() == ( QPair<long double, long double>( 12.0, 130.0 ) ) );
    tsplot->setXRange ( 13.0, 110.0 );
    QVERIFY( tsplot->getXRange() == ( QPair<long double, long double>( 13.0, 110.0 ) ) );
}


#include "XRangeTest.moc"
// kate: indent-mode cstyle; space-indent on; indent-width 0; 
