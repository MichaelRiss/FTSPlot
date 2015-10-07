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
#include "EventsTest.h"

void EventsTest::initTestCase()
{
    QDir testDir;

    tsplot = new FTSPlotWidget ( NULL );
    QVERIFY ( tsplot != NULL );
    tsplot->show();

    testDir.setPath ( "testData/EventEditorTest" );
    QVERIFY ( testDir.exists() == true );
    eeModule = (EventEditor*) tsplot->addEventEditorModule ( "testData/EventEditorTest" );
    QVERIFY ( eeModule != NULL );
}

void EventsTest::cleanupTestCase()
{
    QVERIFY( eeModule->closeTreeDir() );
  
    bool result;
    result = tsplot->deleteModule ( eeModule );
    QVERIFY ( result == true );

    delete tsplot;
}

void EventsTest::eventTests()
{ 
    QVERIFY( eeModule->getCurrentEvent() == 0 );
  
    // add Event(1)
    QVERIFY( eeModule->addEvent( 4 ) );
    QVERIFY( eeModule->getCurrentEvent() == 4 );

    // hasEvent(1)
    QVERIFY( eeModule->hasEvent( 4 ) );
    QVERIFY( !eeModule->hasEvent( 5 ) );
     
    // add Event(2)
    QVERIFY( eeModule->addEvent( 9 ) );
    QVERIFY( eeModule->getCurrentEvent() == 4 );
    
    // hasEvent(2)
    QVERIFY( eeModule->hasEvent( 4 ) );
    QVERIFY( !eeModule->hasEvent( 5 ) );
    QVERIFY( eeModule->hasEvent( 9 ) );
    
    // hasNextEvent(1)
    QVERIFY( eeModule->hasNextEvent( 4 ) );
    QVERIFY( !eeModule->hasPrevEvent( 4 ) );
    
    //getNextEvent(1)
    QVERIFY( eeModule->nextEvent( 4 ) == 9 );
    
    // hasPrevEvent(2)
    QVERIFY( !eeModule->hasNextEvent( 9 ) );
    QVERIFY( eeModule->hasPrevEvent( 9 ) );
    
    // getPrevEvent(2)
    QVERIFY( eeModule->prevEvent( 9 ) == 4 );
    
    QVERIFY( eeModule->getCurrentEvent() == 4 );
    
    QVERIFY( eeModule->setCurrentEvent( 5 ) == false );
    QVERIFY( eeModule->getCurrentEvent() == 4 );
    QVERIFY( eeModule->setCurrentEvent( 9 ) == true );
    QVERIFY( eeModule->getCurrentEvent() == 9 );
    
}


#include "EventsTest.moc"
