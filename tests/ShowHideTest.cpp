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


#include "ShowHideTest.h"

void ShowHideTest::initTestCase()
{
    tsplot = new FTSPlotWidget ( NULL );
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

void ShowHideTest::cleanupTestCase()
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

void ShowHideTest::runtest()
{
    QVERIFY( tsplot->isModuleVisible( tsModule ) == true );
    QVERIFY( tsplot->isModuleVisible( eeModule ) == true );
    QVERIFY( tsplot->isModuleVisible( ieModule ) == true );
    
    QVERIFY( tsplot->isModuleGUIVisible( tsModule ) == false );
    QVERIFY( tsplot->isModuleGUIVisible( eeModule ) == true );
    QVERIFY( tsplot->isModuleGUIVisible( ieModule ) == true );
    
    QVERIFY( tsplot->hideModule ( tsModule ) );
    
    QVERIFY( tsplot->isModuleVisible( tsModule ) == false );
    QVERIFY( tsplot->isModuleVisible( eeModule ) == true );
    QVERIFY( tsplot->isModuleVisible( ieModule ) == true );
    
    QVERIFY( tsplot->isModuleGUIVisible( tsModule ) == false );
    QVERIFY( tsplot->isModuleGUIVisible( eeModule ) == true );
    QVERIFY( tsplot->isModuleGUIVisible( ieModule ) == true );
    
    QVERIFY( tsplot->showModule ( tsModule ) );
    
    QVERIFY( tsplot->isModuleVisible( tsModule ) == true );
    QVERIFY( tsplot->isModuleVisible( eeModule ) == true );
    QVERIFY( tsplot->isModuleVisible( ieModule ) == true );
    
    QVERIFY( tsplot->isModuleGUIVisible( tsModule ) == false );
    QVERIFY( tsplot->isModuleGUIVisible( eeModule ) == true );
    QVERIFY( tsplot->isModuleGUIVisible( ieModule ) == true );
    
    QVERIFY( tsplot->hideModule ( eeModule ) );
    
    QVERIFY( tsplot->isModuleVisible( tsModule ) == true );
    QVERIFY( tsplot->isModuleVisible( eeModule ) == false );
    QVERIFY( tsplot->isModuleVisible( ieModule ) == true );
    
    QVERIFY( tsplot->isModuleGUIVisible( tsModule ) == false );
    QVERIFY( tsplot->isModuleGUIVisible( eeModule ) == true );
    QVERIFY( tsplot->isModuleGUIVisible( ieModule ) == true );
    
    QVERIFY( tsplot->showModule ( eeModule ) );
    
    QVERIFY( tsplot->isModuleVisible( tsModule ) == true );
    QVERIFY( tsplot->isModuleVisible( eeModule ) == true );
    QVERIFY( tsplot->isModuleVisible( ieModule ) == true );
    
    QVERIFY( tsplot->isModuleGUIVisible( tsModule ) == false );
    QVERIFY( tsplot->isModuleGUIVisible( eeModule ) == true );
    QVERIFY( tsplot->isModuleGUIVisible( ieModule ) == true );
    
    QVERIFY( tsplot->hideModule ( ieModule ) );
    
    QVERIFY( tsplot->isModuleVisible( tsModule ) == true );
    QVERIFY( tsplot->isModuleVisible( eeModule ) == true );
    QVERIFY( tsplot->isModuleVisible( ieModule ) == false );
    
    QVERIFY( tsplot->isModuleGUIVisible( tsModule ) == false );
    QVERIFY( tsplot->isModuleGUIVisible( eeModule ) == true );
    QVERIFY( tsplot->isModuleGUIVisible( ieModule ) == true );
    
    QVERIFY( tsplot->showModule ( ieModule ) );
    
    QVERIFY( tsplot->isModuleVisible( tsModule ) == true );
    QVERIFY( tsplot->isModuleVisible( eeModule ) == true );
    QVERIFY( tsplot->isModuleVisible( ieModule ) == true );
    
    QVERIFY( tsplot->isModuleGUIVisible( tsModule ) == false );
    QVERIFY( tsplot->isModuleGUIVisible( eeModule ) == true );
    QVERIFY( tsplot->isModuleGUIVisible( ieModule ) == true );
    
    
    QVERIFY( tsplot->hideModuleGUI ( tsModule ) );
    
    QVERIFY( tsplot->isModuleVisible( tsModule ) == true );
    QVERIFY( tsplot->isModuleVisible( eeModule ) == true );
    QVERIFY( tsplot->isModuleVisible( ieModule ) == true );
    
    QVERIFY( tsplot->isModuleGUIVisible( tsModule ) == false );
    QVERIFY( tsplot->isModuleGUIVisible( eeModule ) == true );
    QVERIFY( tsplot->isModuleGUIVisible( ieModule ) == true );
    
    QVERIFY( tsplot->showModuleGUI ( tsModule ) );
    
    QVERIFY( tsplot->isModuleVisible( tsModule ) == true );
    QVERIFY( tsplot->isModuleVisible( eeModule ) == true );
    QVERIFY( tsplot->isModuleVisible( ieModule ) == true );
    
    QVERIFY( tsplot->isModuleGUIVisible( tsModule ) == false );
    QVERIFY( tsplot->isModuleGUIVisible( eeModule ) == true );
    QVERIFY( tsplot->isModuleGUIVisible( ieModule ) == true );
    
    QVERIFY( tsplot->hideModuleGUI ( eeModule ) );
    
    QVERIFY( tsplot->isModuleVisible( tsModule ) == true );
    QVERIFY( tsplot->isModuleVisible( eeModule ) == true );
    QVERIFY( tsplot->isModuleVisible( ieModule ) == true );
    
    QVERIFY( tsplot->isModuleGUIVisible( tsModule ) == false );
    QVERIFY( tsplot->isModuleGUIVisible( eeModule ) == false );
    QVERIFY( tsplot->isModuleGUIVisible( ieModule ) == true );
    
    QVERIFY( tsplot->showModuleGUI ( eeModule ) );
    
    QVERIFY( tsplot->isModuleVisible( tsModule ) == true );
    QVERIFY( tsplot->isModuleVisible( eeModule ) == true );
    QVERIFY( tsplot->isModuleVisible( ieModule ) == true );
    
    QVERIFY( tsplot->isModuleGUIVisible( tsModule ) == false );
    QVERIFY( tsplot->isModuleGUIVisible( eeModule ) == true );
    QVERIFY( tsplot->isModuleGUIVisible( ieModule ) == true );
    
    QVERIFY( tsplot->hideModuleGUI ( ieModule ) );
    
    QVERIFY( tsplot->isModuleVisible( tsModule ) == true );
    QVERIFY( tsplot->isModuleVisible( eeModule ) == true );
    QVERIFY( tsplot->isModuleVisible( ieModule ) == true );
    
    QVERIFY( tsplot->isModuleGUIVisible( tsModule ) == false );
    QVERIFY( tsplot->isModuleGUIVisible( eeModule ) == true );
    QVERIFY( tsplot->isModuleGUIVisible( ieModule ) == false );
    
    QVERIFY( tsplot->showModuleGUI ( ieModule ) );
    
    QVERIFY( tsplot->isModuleVisible( tsModule ) == true );
    QVERIFY( tsplot->isModuleVisible( eeModule ) == true );
    QVERIFY( tsplot->isModuleVisible( ieModule ) == true );
    
    QVERIFY( tsplot->isModuleGUIVisible( tsModule ) == false );
    QVERIFY( tsplot->isModuleGUIVisible( eeModule ) == true );
    QVERIFY( tsplot->isModuleGUIVisible( ieModule ) == true );
}


#include "ShowHideTest.moc"
