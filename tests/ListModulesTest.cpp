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


#include "ListModulesTest.h"



void ListModulesTest::initTestCase()
{
    tsplot = new FTSPlotWidget ( NULL );
    QVERIFY ( tsplot != NULL );
    tsplot->show();
}

void ListModulesTest::cleanupTestCase()
{
    delete tsplot;
}

void ListModulesTest::runtest()
{
    QList<vizModule> list;
    
    list = tsplot->listModules();
    QVERIFY( list.size() == 0 );
    
    QDir testDir ( "testData" );
    QVERIFY ( testDir.exists() == true );

    QFile testFile ( "testData/timeSeriesDataFile.bin.cfg" );
    QVERIFY ( testFile.exists() == true );
    tsModule = tsplot->addTimeSeries ( "testData/timeSeriesDataFile.bin.cfg" );
    QVERIFY ( tsModule != NULL );

    list = tsplot->listModules();
    QVERIFY( list.size() == 1 );
    QVERIFY( list[0].module == tsModule );

    testDir.setPath ( "testData/EventEditorTest" );
    QVERIFY ( testDir.exists() == true );
    eeModule = tsplot->addEventEditorModule ( "testData/EventEditorTest" );
    QVERIFY ( eeModule != NULL );

    list = tsplot->listModules();
    QVERIFY( list.size() == 2 );
    QVERIFY( list[0].module == tsModule );
    QVERIFY( list[1].module == eeModule );

    testDir.setPath ( "testData/IntervalEditorTest" );
    QVERIFY ( testDir.exists() == true );
    ieModule = tsplot->addIntervalEditorModule ( "testData/IntervalEditorTest" );
    QVERIFY ( ieModule != NULL );

    list = tsplot->listModules();
    QVERIFY( list.size() == 3 );
    QVERIFY( list[0].module == tsModule );
    QVERIFY( list[1].module == eeModule );
    QVERIFY( list[2].module == ieModule );
    
    bool result;
    result = tsplot->deleteModule ( ieModule );
    QVERIFY ( result == true );

    list = tsplot->listModules();
    QVERIFY( list.size() == 2 );
    QVERIFY( list[0].module == tsModule );
    QVERIFY( list[1].module == eeModule );
    
    result = tsplot->deleteModule ( eeModule );
    QVERIFY ( result == true );

    list = tsplot->listModules();
    QVERIFY( list.size() == 1 );
    QVERIFY( list[0].module == tsModule );
    
    result = tsplot->deleteModule ( tsModule );
    QVERIFY ( result == true );
    
    list = tsplot->listModules();
    QVERIFY( list.size() == 0 );
}


#include "ListModulesTest.moc"
