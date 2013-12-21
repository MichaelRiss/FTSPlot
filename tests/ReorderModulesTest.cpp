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


#include "ReorderModulesTest.h"

void ReorderModulesTest::initTestCase()
{
    tsplot = new SimpleViewWidget ( NULL );
    QVERIFY ( tsplot != NULL );
    tsplot->show();

    QDir testDir ( "testData" );
    QVERIFY ( testDir.exists() == true );

    QFile testFile ( "testData/timeSeriesDataFile.bin.cfg" );
    QVERIFY ( testFile.exists() == true );
    tsModule = tsplot->addTimeSeries ( "testData/timeSeriesDataFile.bin.cfg" );
    QVERIFY ( tsModule != NULL );

    testDir.setPath ( "testData/EventEditorTest" );
    QVERIFY ( testDir.exists() == true );
    eeModule = tsplot->addEventEditorModule ( "testData/EventEditorTest" );
    QVERIFY ( eeModule != NULL );


    testDir.setPath ( "testData/IntervalEditorTest" );
    QVERIFY ( testDir.exists() == true );
    ieModule = tsplot->addIntervalEditorModule ( "testData/IntervalEditorTest" );
    QVERIFY ( ieModule != NULL );
}

void ReorderModulesTest::cleanupTestCase()
{
    bool result;
    result = tsplot->deleteModule ( ieModule );
    QVERIFY ( result == true );

    result = tsplot->deleteModule ( eeModule );
    QVERIFY ( result == true );

    result = tsplot->deleteModule ( tsModule );
    QVERIFY ( result == true );

    delete tsplot;
}

void ReorderModulesTest::reorderTest()
{
    QList<vizModule> modList = tsplot->listModules();
    modList.append( modList.takeFirst() );
    
    QVERIFY( tsplot->reorderModules ( modList ) );
    
    for( int i = 0; i < modList.size(); i++ )
    {
      QVERIFY( modList[i].module == tsplot->listModules()[i].module );
    }
}


#include "ReorderModulesTest.moc"
// kate: indent-mode cstyle; space-indent on; indent-width 0; 
