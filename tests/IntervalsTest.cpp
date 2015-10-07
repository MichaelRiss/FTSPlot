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


#include "IntervalsTest.h"

void IntervalsTest::initTestCase()
{
    QDir testDir;

    tsplot = new FTSPlotWidget ( NULL );
    QVERIFY ( tsplot != NULL );
    tsplot->show();

    testDir.setPath ( "testData/IntervalEditorTest" );
    QVERIFY ( testDir.exists() == true );
    ieModule = ( IntervalEditor* ) tsplot->addIntervalEditorModule ( "testData/IntervalEditorTest" );
    QVERIFY ( ieModule != NULL );
}

void IntervalsTest::cleanupTestCase()
{
    QVERIFY ( ieModule->closeTreeDir() );

    bool result;
    result = tsplot->deleteModule ( ieModule );
    QVERIFY ( result == true );

    delete tsplot;
}

void IntervalsTest::intervalTests()
{
    QVERIFY ( ieModule->getCurrentInterval() == Interval ( 0, 1 ) );

    // add Interval(1)
    QVERIFY ( ieModule->addInterval ( Interval ( 4, 6 ) ) );
    QVERIFY ( ieModule->getCurrentInterval() == Interval ( 4, 6 ) );

    // has Interval(1)
    QVERIFY ( ieModule->hasInterval ( Interval ( 4, 6 ) ) );
    QVERIFY ( !ieModule->hasInterval ( Interval ( 5, 8 ) ) );

    // add Interval(2)
    QVERIFY ( ieModule->addInterval ( Interval ( 9, 14 ) ) );
    QVERIFY ( ieModule->getCurrentInterval() == Interval ( 4, 6 ) );

    // has Inteval(2)
    QVERIFY ( ieModule->hasInterval ( Interval ( 4, 6 ) ) );
    QVERIFY ( !ieModule->hasInterval ( Interval ( 5, 8 ) ) );
    QVERIFY ( ieModule->hasInterval ( Interval ( 9, 14 ) ) );

    // has NextInterval(1)
    QVERIFY ( ieModule->hasNextInterval ( Interval ( 4, 6 ) ) );
    QVERIFY ( !ieModule->hasPrevInterval ( Interval ( 4, 6 ) ) );

    //get NextInterval(1)
    QVERIFY ( ieModule->nextInterval ( Interval ( 4, 6 ) ) == Interval ( 9, 14 ) );

    // has PrevInterval(2)
    QVERIFY ( !ieModule->hasNextInterval ( Interval ( 9, 14 ) ) );
    QVERIFY ( ieModule->hasPrevInterval ( Interval ( 9, 14 ) ) );

    // get PrevInterval(2)
    QVERIFY ( ieModule->prevInterval ( Interval ( 9, 14 ) ) == Interval ( 4, 6 ) );

    QVERIFY ( ieModule->getCurrentInterval() == Interval ( 4, 6 ) );
    QVERIFY ( ieModule->setCurrentInterval ( Interval ( 5, 8 ) ) == false );
    QVERIFY ( ieModule->getCurrentInterval() == Interval ( 4, 6 ) );
    QVERIFY ( ieModule->setCurrentInterval ( Interval ( 9, 14 ) ) == true );
    QVERIFY ( ieModule->getCurrentInterval() == Interval ( 9, 14 ) );
}


#include "IntervalsTest.moc"
